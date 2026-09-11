#!/usr/bin/env python3
"""Render a video of a Lennard-Jones simulated-annealing trajectory.

Reads the multi-frame .xyz trajectory dumped by the Rust program
(one frame per log_interval iterations: an atom count line, a comment
line with iter/energy, then one "Ar x y z" line per atom), renders
each frame to a PNG in parallel across a worker pool, then stitches
the PNGs into an mp4 with a single ffmpeg pass.

Usage:
    python3 render_trajectory.py trajectory_n80_3d.xyz -o annealing.mp4 -j 8

Or:
    uv run --with matplotlib render_trajectory.py trajectory_n80_3d.xyz -o annealing.mp4 -j 8
"""
import argparse
import multiprocessing as mp
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np

COMMENT_RE = re.compile(
    r"iter=(?P<iter>\d+)"
    r"(?:\s+E=(?P<energy>[\-+0-9.eE]+))?"
    r"(?:\s+best=(?P<best>[\-+0-9.eE]+))?"
)


def parse_trajectory(path):
    """Parse a multi-frame .xyz file into a list of frames.

    Each frame is a dict with keys: positions (N,3 float array), and
    optionally iter/energy/best parsed from the comment line.
    """
    frames = []
    with open(path) as f:
        while True:
            header = f.readline()
            if not header:
                break
            header = header.strip()
            if not header:
                continue
            n = int(header)
            comment = f.readline().strip()
            positions = np.empty((n, 3), dtype=float)
            for i in range(n):
                parts = f.readline().split()
                positions[i] = [float(parts[1]), float(parts[2]), float(parts[3])]

            meta = {"positions": positions, "comment": comment}
            m = COMMENT_RE.search(comment)
            if m:
                meta["iter"] = int(m.group("iter"))
                meta["energy"] = m.group("energy")
                meta["best"] = m.group("best")
            frames.append(meta)
    if not frames:
        raise ValueError(f"no frames found in {path}")
    return frames


def detect_dimension(frames, z_tol=1e-9):
    for frame in frames:
        if np.any(np.abs(frame["positions"][:, 2]) > z_tol):
            return 3
    return 2


def axis_bounds(frames, dim, padding=0.15):
    all_pts = np.concatenate([f["positions"][:, :dim] for f in frames], axis=0)
    lo = all_pts.min(axis=0)
    hi = all_pts.max(axis=0)
    span = np.maximum(hi - lo, 1e-6)
    lo = lo - padding * span
    hi = hi + padding * span
    return lo, hi


def frame_title(frame):
    bits = [f"iter {frame.get('iter', '?')}"]
    if frame.get("energy"):
        bits.append(f"E = {float(frame['energy']):.4e}")
    if frame.get("best"):
        bits.append(f"best = {float(frame['best']):.4e}")
    return " | ".join(bits)


def format_duration(seconds):
    """Formats a duration in seconds as `HH:MM:SS`."""
    if seconds is None or seconds != seconds or seconds < 0 or seconds == float("inf"):
        return "n/a"
    total = round(seconds)
    h, rem = divmod(total, 3600)
    m, s = divmod(rem, 60)
    return f"{h:02}:{m:02}:{s:02}"


def draw_2d(ax, frame, lo, hi):
    pts = frame["positions"][:, :2]
    ax.scatter(pts[:, 0], pts[:, 1], s=120, c="tab:blue", edgecolors="k", zorder=3)
    ax.set_xlim(lo[0], hi[0])
    ax.set_ylim(lo[1], hi[1])
    ax.set_aspect("equal")


def draw_3d(ax, frame, lo, hi, azim):
    pts = frame["positions"]
    ax.scatter(pts[:, 0], pts[:, 1], pts[:, 2], s=80, c="tab:blue", edgecolors="k")
    ax.set_xlim(lo[0], hi[0])
    ax.set_ylim(lo[1], hi[1])
    ax.set_zlim(lo[2], hi[2])
    ax.view_init(elev=20, azim=azim)


# Populated once per worker process by _init_worker; avoids re-creating the
# figure/axes (expensive) for every frame that worker renders.
_worker = {}


def _init_worker(dim, lo, hi, rotate, total_frames, out_dir, dpi):
    if dim == 2:
        fig, ax = plt.subplots(figsize=(7, 7))
    else:
        fig = plt.figure(figsize=(7, 7))
        ax = fig.add_subplot(projection="3d")
    _worker.update(
        dim=dim, lo=lo, hi=hi, rotate=rotate, total_frames=total_frames,
        fig=fig, ax=ax, out_dir=out_dir, dpi=dpi,
    )


def _render_frame(item):
    i, frame = item
    ax = _worker["ax"]
    ax.clear()
    if _worker["dim"] == 2:
        draw_2d(ax, frame, _worker["lo"], _worker["hi"])
    else:
        azim = (i * 360 / _worker["total_frames"]) % 360 if _worker["rotate"] else -60
        draw_3d(ax, frame, _worker["lo"], _worker["hi"], azim)
    ax.set_title(frame_title(frame), fontsize=10)
    out_path = os.path.join(_worker["out_dir"], f"frame_{i:06d}.png")
    _worker["fig"].savefig(out_path, dpi=_worker["dpi"])
    return i


def encode_video(frame_dir, out_path, fps):
    if shutil.which("ffmpeg") is None:
        raise RuntimeError("ffmpeg not found on PATH; install it to encode the video")
    cmd = [
        "ffmpeg", "-y",
        "-framerate", str(fps),
        "-i", os.path.join(frame_dir, "frame_%06d.png"),
        # libx264 requires even width/height; pad up if dpi * figsize is odd.
        "-vf", "pad=ceil(iw/2)*2:ceil(ih/2)*2",
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        out_path,
    ]
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    if result.returncode != 0:
        raise RuntimeError(f"ffmpeg failed:\n{result.stderr.decode(errors='replace')}")


def render_video(frames, out_path, fps, dpi, dim, rotate, jobs):
    lo, hi = axis_bounds(frames, dim)
    total = len(frames)
    jobs = max(1, min(jobs, total))

    with tempfile.TemporaryDirectory(prefix="lj_frames_") as tmp_dir:
        start = time.monotonic()
        done = 0
        with mp.Pool(
            processes=jobs,
            initializer=_init_worker,
            initargs=(dim, lo, hi, rotate, total, tmp_dir, dpi),
        ) as pool:
            for _ in pool.imap_unordered(_render_frame, enumerate(frames), chunksize=1):
                done += 1
                elapsed = time.monotonic() - start
                throughput = done / elapsed if elapsed > 0 else float("inf")
                eta = format_duration((total - done) / throughput) if throughput > 0 else "n/a"
                end = "\n" if done >= total else "\r"
                print(
                    f"frame={done:>5}/{total} ({100 * done / total:5.1f}%) "
                    f"| jobs={jobs} | throughput={throughput:6.2f} fps | ETA={eta}",
                    end=end,
                    file=sys.stderr,
                    flush=True,
                )

        encode_start = time.monotonic()
        encode_video(tmp_dir, out_path, fps)
        print(
            f"encoded {out_path} | elapsed={format_duration(time.monotonic() - encode_start)}",
            file=sys.stderr,
        )


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("trajectory", help="path to the trajectory .xyz file")
    parser.add_argument("-o", "--output", default="annealing.mp4", help="output video path")
    parser.add_argument("--fps", type=int, default=15, help="frames per second")
    parser.add_argument("--dpi", type=int, default=150, help="output resolution (dots per inch)")
    parser.add_argument("--dim", type=int, choices=(2, 3), default=None, help="force dimensionality (default: auto-detect)")
    parser.add_argument("--no-rotate", action="store_true", help="disable slow camera rotation in 3D renders")
    parser.add_argument(
        "-j", "--jobs", type=int, default=os.cpu_count() or 1,
        help="number of worker processes to render frames in parallel (default: all CPUs)",
    )
    args = parser.parse_args()

    parse_start = time.monotonic()
    frames = parse_trajectory(args.trajectory)
    dim = args.dim or detect_dimension(frames)
    print(
        f"parsed {len(frames)} frames | dim={dim} | elapsed={format_duration(time.monotonic() - parse_start)}",
        file=sys.stderr,
    )

    render_start = time.monotonic()
    render_video(frames, args.output, args.fps, args.dpi, dim, rotate=not args.no_rotate, jobs=args.jobs)
    print(
        f"wrote {args.output} | elapsed={format_duration(time.monotonic() - render_start)}",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
