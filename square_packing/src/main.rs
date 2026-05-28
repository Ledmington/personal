#![forbid(unsafe_code)]

use std::{f64::consts::PI, fs};

use plotters::prelude::*;
use rand::{RngExt, SeedableRng, rngs::ChaCha8Rng};

const SMALL_SQUARE_SIDE: f64 = 1.0;

#[derive(Clone, Copy)]
struct Point {
    x: f64,
    y: f64,
}

#[derive(Clone)]
struct ImplicitSquare {
    center: Point,
    theta: f64,
}

struct ExplicitSquare {
    vertices: [Point; 4],
}

struct PackingResult {
    total_overlap: f64,
    big_square_side: f64,
}

fn init_random(seed: u64, n_squares: usize, limit: f64) -> Vec<ImplicitSquare> {
    let mut rng = ChaCha8Rng::seed_from_u64(seed);
    let mut squares: Vec<ImplicitSquare> = Vec::with_capacity(n_squares);
    for _ in 0..n_squares {
        squares.push(ImplicitSquare {
            center: Point {
                x: rng.random_range(0.0..limit),
                y: rng.random_range(0.0..limit),
            },
            theta: rng.random_range(0.0..(2.0 * PI)),
        });
    }
    squares
}

fn rotate(p: Point, theta: f64) -> Point {
    let cos_t = theta.cos();
    let sin_t = theta.sin();

    Point {
        x: p.x * cos_t - p.y * sin_t,
        y: p.x * sin_t + p.y * cos_t,
    }
}

fn implicit_to_explicit(square: &ImplicitSquare) -> ExplicitSquare {
    let h = SMALL_SQUARE_SIDE / 2.0;

    // Local square corners (counter-clockwise)
    let local = [
        Point { x: -h, y: -h }, // p1
        Point { x: h, y: -h },  // p2
        Point { x: h, y: h },   // p3
        Point { x: -h, y: h },  // p4
    ];

    let mut pts = local.map(|p| rotate(p, square.theta));

    // translate to center
    for p in &mut pts {
        p.x += square.center.x;
        p.y += square.center.y;
    }

    ExplicitSquare { vertices: pts }
}

fn polygon_area(poly: &[(f64, f64)]) -> f64 {
    let n = poly.len();
    let mut area = 0.0;

    for i in 0..n {
        let (x1, y1) = poly[i];
        let (x2, y2) = poly[(i + 1) % n];
        area += x1 * y2 - x2 * y1;
    }

    area.abs() * 0.5
}

fn inside(p: (f64, f64), a: (f64, f64), b: (f64, f64)) -> bool {
    (b.0 - a.0) * (p.1 - a.1) - (b.1 - a.1) * (p.0 - a.0) >= 0.0
}

fn intersection(a1: (f64, f64), a2: (f64, f64), b1: (f64, f64), b2: (f64, f64)) -> (f64, f64) {
    let da = (a2.0 - a1.0, a2.1 - a1.1);
    let db = (b2.0 - b1.0, b2.1 - b1.1);
    let dp = (a1.0 - b1.0, a1.1 - b1.1);

    let det = da.0 * db.1 - da.1 * db.0;
    if det.abs() < 1e-12 {
        return a1;
    }

    let t = (dp.0 * db.1 - dp.1 * db.0) / det;

    (a1.0 + t * da.0, a1.1 + t * da.1)
}

fn clip_polygon(subject: Vec<(f64, f64)>, clip: &[(f64, f64)]) -> Vec<(f64, f64)> {
    let mut output = subject;

    for i in 0..clip.len() {
        let a = clip[i];
        let b = clip[(i + 1) % clip.len()];

        let input = output;
        output = Vec::new();

        if input.is_empty() {
            break;
        }

        let mut s = input[input.len() - 1];

        for &e in &input {
            if inside(e, a, b) {
                if !inside(s, a, b) {
                    output.push(intersection(s, e, a, b));
                }
                output.push(e);
            } else if inside(s, a, b) {
                output.push(intersection(s, e, a, b));
            }
            s = e;
        }
    }

    output
}

fn evaluate(squares: &[ImplicitSquare]) -> PackingResult {
    let mut total_overlap = 0.0;

    // collect all vertices for bounding square
    let mut min_x = f64::INFINITY;
    let mut min_y = f64::INFINITY;
    let mut max_x = f64::NEG_INFINITY;
    let mut max_y = f64::NEG_INFINITY;

    let explicit: Vec<_> = squares.iter().map(implicit_to_explicit).collect();

    for s in &explicit {
        for p in &s.vertices {
            min_x = min_x.min(p.x);
            min_y = min_y.min(p.y);
            max_x = max_x.max(p.x);
            max_y = max_y.max(p.y);
        }
    }

    // pairwise overlap (approx)
    for i in 0..explicit.len() {
        let a = explicit[i].vertices.map(|p| (p.x, p.y)).to_vec();

        for j in i + 1..explicit.len() {
            let b = explicit[j].vertices.map(|p| (p.x, p.y)).to_vec();

            let clipped = clip_polygon(a.clone(), &b);
            if clipped.len() >= 3 {
                total_overlap += polygon_area(&clipped);
            }
        }
    }

    let big_square_side = (max_x - min_x).max(max_y - min_y);

    PackingResult {
        total_overlap,
        big_square_side,
    }
}

fn translate_to_origin(squares: &mut [ImplicitSquare]) {
    // First collect all explicit vertices
    let mut min_x = f64::INFINITY;
    let mut min_y = f64::INFINITY;

    // We recompute geometry to find bounding box
    for sq in squares.iter() {
        let exp = implicit_to_explicit(sq);

        for v in exp.vertices.iter() {
            min_x = min_x.min(v.x);
            min_y = min_y.min(v.y);
        }
    }

    let dx = -min_x;
    let dy = -min_y;

    // Apply rigid translation to all centers
    for sq in squares.iter_mut() {
        sq.center.x += dx;
        sq.center.y += dy;
    }
}

fn apply_forces_only_repulsion(squares: &mut Vec<ImplicitSquare>) {
    let n = squares.len();
    let mut deltas = vec![(0.0f64, 0.0f64); n];

    let explicit: Vec<_> = squares.iter().map(implicit_to_explicit).collect();

    for i in 0..n {
        for j in i + 1..n {
            let a = explicit[i].vertices.map(|p| (p.x, p.y)).to_vec();
            let b = explicit[j].vertices.map(|p| (p.x, p.y)).to_vec();

            let clipped = clip_polygon(a, &b);
            if clipped.len() < 3 {
                continue;
            }

            let overlap = polygon_area(&clipped);
            if overlap <= 1e-12 {
                continue;
            }

            let dx = squares[i].center.x - squares[j].center.x;
            let dy = squares[i].center.y - squares[j].center.y;
            let dist = (dx * dx + dy * dy).sqrt().max(1e-6);

            let fx = dx / dist * overlap;
            let fy = dy / dist * overlap;

            deltas[i].0 += fx;
            deltas[i].1 += fy;
            deltas[j].0 -= fx;
            deltas[j].1 -= fy;
        }
    }

    // apply updates (small step to avoid explosion)
    let step = 0.05;

    for i in 0..n {
        squares[i].center.x += deltas[i].0 * step;
        squares[i].center.y += deltas[i].1 * step;
    }
}

fn apply_forces_repulsion_and_attraction(squares: &mut Vec<ImplicitSquare>) {
    let n = squares.len();
    let mut deltas = vec![(0.0f64, 0.0f64); n];

    let explicit: Vec<_> = squares.iter().map(implicit_to_explicit).collect();

    for i in 0..n {
        for j in (i + 1)..n {
            let dx = squares[i].center.x - squares[j].center.x;
            let dy = squares[i].center.y - squares[j].center.y;

            let dist_sq = dx * dx + dy * dy;
            let dist = dist_sq.sqrt().max(1e-6);

            let ux = dx / dist;
            let uy = dy / dist;

            let a = explicit[i].vertices.map(|p| (p.x, p.y)).to_vec();
            let b = explicit[j].vertices.map(|p| (p.x, p.y)).to_vec();

            let clipped = clip_polygon(a, &b);

            let mut fx = 0.0;
            let mut fy = 0.0;

            if clipped.len() >= 3 {
                // REPULSION (overlap)
                let overlap = polygon_area(&clipped);

                if overlap > 1e-12 {
                    let strength = overlap;
                    fx += ux * strength;
                    fy += uy * strength;
                }
            } else {
                // ATTRACTION (inverse square law)
                let strength = 0.02 / (dist_sq + 1e-6);
                fx -= ux * strength;
                fy -= uy * strength;
            }

            deltas[i].0 += fx;
            deltas[i].1 += fy;
            deltas[j].0 -= fx;
            deltas[j].1 -= fy;
        }
    }

    // damped update for stability
    let step = 0.03;

    for i in 0..n {
        squares[i].center.x += deltas[i].0 * step;
        squares[i].center.y += deltas[i].1 * step;
    }
}

fn apply_forces_repulsion_and_attraction_to_origin(squares: &mut Vec<ImplicitSquare>) {
    let n = squares.len();
    let mut deltas = vec![(0.0f64, 0.0f64); n];

    let explicit: Vec<_> = squares.iter().map(implicit_to_explicit).collect();

    // pairwise repulsion only (no attraction between squares anymore)
    for i in 0..n {
        for j in i + 1..n {
            let dx = squares[i].center.x - squares[j].center.x;
            let dy = squares[i].center.y - squares[j].center.y;

            let dist = (dx * dx + dy * dy).sqrt().max(1e-6);
            let ux = dx / dist;
            let uy = dy / dist;

            let a = explicit[i].vertices.map(|p| (p.x, p.y)).to_vec();
            let b = explicit[j].vertices.map(|p| (p.x, p.y)).to_vec();

            let clipped = clip_polygon(a, &b);

            if clipped.len() >= 3 {
                let overlap = polygon_area(&clipped);

                if overlap > 1e-12 {
                    let strength = overlap;

                    deltas[i].0 += ux * strength;
                    deltas[i].1 += uy * strength;

                    deltas[j].0 -= ux * strength;
                    deltas[j].1 -= uy * strength;
                }
            }
        }
    }

    // attraction to origin (stable spring force)
    let origin_k = 0.1;

    for i in 0..n {
        let cx = squares[i].center.x;
        let cy = squares[i].center.y;

        deltas[i].0 += -origin_k * cx;
        deltas[i].1 += -origin_k * cy;
    }

    // apply updates (damped)
    let step = 0.03;

    for i in 0..n {
        squares[i].center.x += deltas[i].0 * step;
        squares[i].center.y += deltas[i].1 * step;
    }
}

fn rotate_one(squares: &mut Vec<ImplicitSquare>) {
    let explicit = squares.iter().map(implicit_to_explicit).collect::<Vec<_>>();
    let n = squares.len();

    let mut worst_idx = 0;
    let mut worst_score = 0.0;

    // find worst square
    for i in 0..n {
        let mut score = 0.0;

        let a = explicit[i].vertices.map(|p| (p.x, p.y)).to_vec();

        for j in 0..n {
            if i == j {
                continue;
            }

            let b = explicit[j].vertices.map(|p| (p.x, p.y)).to_vec();
            let clipped = clip_polygon(a.clone(), &b);

            if clipped.len() >= 3 {
                score += polygon_area(&clipped);
            }
        }

        if score > worst_score {
            worst_score = score;
            worst_idx = i;
        }
    }

    // brute-force best rotation
    let mut best_theta = squares[worst_idx].theta;
    let mut best_score = worst_score;

    let original_center = squares[worst_idx].center;

    for deg in 0..90 {
        let theta = (deg as f64).to_radians();

        squares[worst_idx].theta = theta;

        let explicit = squares.iter().map(implicit_to_explicit).collect::<Vec<_>>();

        let mut score = 0.0;

        let a = explicit[worst_idx].vertices.map(|p| (p.x, p.y)).to_vec();

        for j in 0..n {
            if j == worst_idx {
                continue;
            }

            let b = explicit[j].vertices.map(|p| (p.x, p.y)).to_vec();
            let clipped = clip_polygon(a.clone(), &b);

            if clipped.len() >= 3 {
                score += polygon_area(&clipped);
            }
        }

        if score < best_score {
            best_score = score;
            best_theta = theta;
        }

        // restore center (theta test doesn't change center anyway)
        squares[worst_idx].center = original_center;
    }

    squares[worst_idx].theta = best_theta;
}

fn rotate_all(squares: &mut Vec<ImplicitSquare>) {
    if squares.is_empty() {
        return;
    }

    // --- compute center of mass ---
    let mut cx = 0.0;
    let mut cy = 0.0;

    for s in squares.iter() {
        cx += s.center.x;
        cy += s.center.y;
    }

    let n = squares.len() as f64;
    cx /= n;
    cy /= n;

    let mut best_theta = 0.0;
    let mut best_score = f64::INFINITY;

    // try angles 0..PI/2 in 1 degree steps
    let steps = 90;

    for i in 0..=steps {
        let theta = (i as f64).to_radians();

        let cos_t = theta.cos();
        let sin_t = theta.sin();

        let mut min_x = f64::INFINITY;
        let mut min_y = f64::INFINITY;
        let mut max_x = f64::NEG_INFINITY;
        let mut max_y = f64::NEG_INFINITY;

        // simulate rotation of each square
        for s in squares.iter() {
            // rotate the center around COM
            let dx = s.center.x - cx;
            let dy = s.center.y - cy;
            let new_center_x = cx + dx * cos_t - dy * sin_t;
            let new_center_y = cy + dx * sin_t + dy * cos_t;

            // apply the combined orientation (s.theta + theta) to the local corners
            let combined_theta = s.theta + theta;
            let h = SMALL_SQUARE_SIDE / 2.0;

            let local = [
                Point { x: -h, y: -h },
                Point { x: h, y: -h },
                Point { x: h, y: h },
                Point { x: -h, y: h },
            ];

            for lp in local {
                let rotated = rotate(lp, combined_theta);
                let gx = new_center_x + rotated.x;
                let gy = new_center_y + rotated.y;

                min_x = min_x.min(gx);
                min_y = min_y.min(gy);
                max_x = max_x.max(gx);
                max_y = max_y.max(gy);
            }
        }

        let side = (max_x - min_x).max(max_y - min_y);

        if side < best_score {
            best_score = side;
            best_theta = theta;
        }
    }

    // --- apply best rotation permanently ---
    let cos_t = best_theta.cos();
    let sin_t = best_theta.sin();

    for s in squares.iter_mut() {
        let dx = s.center.x - cx;
        let dy = s.center.y - cy;

        s.center.x = cx + dx * cos_t - dy * sin_t;
        s.center.y = cy + dx * sin_t + dy * cos_t;

        s.theta += best_theta;
    }
}

fn create_plot(
    squares: &Vec<ImplicitSquare>,
    upper_bound: f64,
    packing_result: PackingResult,
    filename: String,
) -> Result<(), Box<dyn std::error::Error>> {
    // Create drawing area
    let root = BitMapBackend::new(&filename, (900, 900)).into_drawing_area();

    root.fill(&WHITE)?;

    // Strictly bounded axes
    let mut chart = ChartBuilder::on(&root)
        .margin(20)
        .caption("Rotated Squares", ("sans-serif", 30))
        .x_label_area_size(40)
        .y_label_area_size(40)
        .build_cartesian_2d(0.0..upper_bound, 0.0..upper_bound)
        .expect("chart")
        .set_secondary_coord(0.0..upper_bound, 0.0..upper_bound);

    chart.configure_mesh().x_desc("X").y_desc("Y").draw()?;

    let base_color = RGBColor(30, 144, 255);

    for (idx, square) in squares.iter().enumerate() {
        let explicit = implicit_to_explicit(square);

        // Different shades of same color
        let mix = 0.25 + (idx as f64) * 0.15;
        let fill_style = base_color.mix(mix.min(0.9)).filled();

        let poly: Vec<(f64, f64)> = vec![
            explicit.vertices[0],
            explicit.vertices[1],
            explicit.vertices[2],
            explicit.vertices[3],
            explicit.vertices[0],
        ]
        .iter()
        .map(|p| (p.x, p.y))
        .collect();

        chart.draw_series(std::iter::once(Polygon::new(poly.clone(), fill_style)))?;

        chart.draw_series(std::iter::once(PathElement::new(
            poly,
            BLACK.stroke_width(2),
        )))?;

        // Draw label at center
        chart.draw_series(std::iter::once(Text::new(
            format!("{}", idx),
            (square.center.x, square.center.y),
            ("sans-serif", 22).into_font().color(&BLACK),
        )))?;
    }

    // BIG bounding square (anchored at origin)
    let big = vec![
        (0.0, 0.0),
        (packing_result.big_square_side, 0.0),
        (
            packing_result.big_square_side,
            packing_result.big_square_side,
        ),
        (0.0, packing_result.big_square_side),
        (0.0, 0.0),
    ];

    chart.draw_series(std::iter::once(PathElement::new(
        big.clone(),
        RED.stroke_width(3),
    )))?;

    {
        let label_x = upper_bound * 0.02;
        let label_y = upper_bound * 0.98;
        let line_gap = upper_bound * 0.04;

        chart.draw_series(std::iter::once(Text::new(
            format!("Total Overlap : {:.10}", packing_result.total_overlap),
            (label_x, label_y),
            ("sans-serif", 20).into_font().color(&BLACK),
        )))?;

        chart.draw_series(std::iter::once(Text::new(
            format!("Big Square Side : {:.10}", packing_result.big_square_side),
            (label_x, label_y - line_gap),
            ("sans-serif", 20).into_font().color(&BLACK),
        )))?;
    }

    root.present()?;

    println!("Saved plot to {}", filename);

    Ok(())
}

fn frames_to_video(
    frame_dir: &str,
    output: &str,
    fps: u32,
) -> Result<(), Box<dyn std::error::Error>> {
    let status = std::process::Command::new("ffmpeg")
        .args([
            "-y", // overwrite output
            "-framerate",
            &fps.to_string(),
            "-i",
            &format!("{}/frame_%04d.png", frame_dir),
            "-c:v",
            "libx264", // or libx265, librav1e, etc.
            "-pix_fmt",
            "yuv420p", // required for compatibility
            output,
        ])
        .status()?;

    if !status.success() {
        return Err(format!("ffmpeg exited with status {}", status).into());
    }
    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let n_squares = 12;

    let lower_bound: f64 = (n_squares as f64).sqrt().floor();
    let upper_bound: f64 = (n_squares as f64).sqrt().ceil();

    println!("Num. Squares : {}", n_squares);
    println!("Lower Bound  : {}", lower_bound);
    println!("Upper Bound  : {}", upper_bound);

    fs::create_dir_all("frames")?;

    let mut squares = init_random(42, n_squares, upper_bound);
    translate_to_origin(&mut squares);

    let max_iterations = 10_000;
    for it in 0..max_iterations {
        apply_forces_repulsion_and_attraction_to_origin(&mut squares);
        rotate_one(&mut squares);
        translate_to_origin(&mut squares);
        rotate_all(&mut squares);
        translate_to_origin(&mut squares);

        let result = evaluate(&squares);
        create_plot(
            &squares,
            5.0,
            // upper_bound.max(result.big_square_side),
            result,
            format!("frames/frame_{:04}.png", it),
        )?;
    }

    frames_to_video("frames", "output.mp4", 30)?;

    Ok(())
}
