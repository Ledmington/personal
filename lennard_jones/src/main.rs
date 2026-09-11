use std::fs::File;
use std::io::{BufWriter, Result as IoResult, Write};
use std::path::PathBuf;
use std::time::Instant;

use clap::{Parser, ValueEnum};
use rand::rngs::StdRng;
use rand::{Rng, RngExt, SeedableRng};
use rand_distr::{Distribution, Normal};

const SIGMA: f64 = 1.0;
const EPSILON: f64 = 1.0;
const STEP_SIZE: f64 = 0.0001;
const GD_LEARNING_RATE: f64 = 0.001;

#[derive(Copy, Clone, Debug, PartialEq, Eq, ValueEnum)]
enum Dimension {
    #[value(name = "2")]
    Two,
    #[value(name = "3")]
    Three,
}

impl Dimension {
    fn as_usize(self) -> usize {
        match self {
            Dimension::Two => 2,
            Dimension::Three => 3,
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq, ValueEnum)]
enum Algorithm {
    #[value(name = "sa")]
    SimulatedAnnealing,
    #[value(name = "gd")]
    GradientDescent,
}

impl Algorithm {
    fn as_str(self) -> &'static str {
        match self {
            Algorithm::SimulatedAnnealing => "sa",
            Algorithm::GradientDescent => "gd",
        }
    }
}

/// Lennard-Jones cluster optimization via simulated annealing or gradient descent.
#[derive(Parser, Debug)]
#[command(about = "Lennard-Jones cluster optimization via simulated annealing or gradient descent")]
struct Cli {
    /// Number of particles in the cluster.
    #[arg(short = 'n', long, default_value_t = 80)]
    n_points: usize,

    /// Spatial dimensionality of the simulation.
    #[arg(short, long, value_enum, default_value_t = Dimension::Three)]
    dim: Dimension,

    /// Optimization algorithm to run.
    #[arg(short = 'a', long, value_enum, default_value_t = Algorithm::SimulatedAnnealing)]
    algorithm: Algorithm,

    /// Number of optimization steps.
    #[arg(short = 'i', long, default_value_t = 10_000_000)]
    max_iters: u64,

    /// Log progress (and dump a trajectory frame) every N iterations.
    #[arg(short = 'l', long, default_value_t = 10_000)]
    log_interval: u64,

    /// Directory where output files (final cluster + trajectory) are written.
    #[arg(short = 'o', long, default_value = ".")]
    output_dir: PathBuf,

    /// RNG seed. If omitted, a random seed is generated and logged.
    #[arg(short = 's', long)]
    seed: Option<u64>,

    /// Gradient descent only: stop early once the gradient's L2 norm drops
    /// below this threshold.
    #[arg(short = 'g', long, default_value_t = 1e-6)]
    gd_tol: f64,
}

#[derive(Clone)]
struct Point {
    x: f64,
    y: f64,
    z: f64,
}

fn lennard_jones(a: &Point, b: &Point) -> f64 {
    let dx = a.x - b.x;
    let dy = a.y - b.y;
    let dz = a.z - b.z;
    let r_ij = (dx * dx + dy * dy + dz * dz).sqrt();
    let r_ij_star = SIGMA / r_ij;
    let r_ij_star2 = r_ij_star * r_ij_star;
    let r_ij_star4 = r_ij_star2 * r_ij_star2;
    let r_ij_star6 = r_ij_star4 * r_ij_star2;
    let r_ij_star12 = r_ij_star6 * r_ij_star6;
    4.0 * EPSILON * (r_ij_star12 - r_ij_star6)
}

fn energy(points: &[Point]) -> f64 {
    let mut s = 0.0;
    for i in 0..points.len() {
        for j in (i + 1)..points.len() {
            s += lennard_jones(&points[i], &points[j]);
        }
    }
    s
}

fn update<R: Rng + ?Sized>(rng: &mut R, points: &mut [Point], dim: Dimension) {
    let normal = Normal::new(0.0, 1.0).unwrap();
    let mut disp = vec![[0.0_f64; 3]; points.len()];
    let mut norm = 0.0;
    for i in 0..points.len() {
        let vx = normal.sample(rng);
        let vy = normal.sample(rng);
        let vz = if dim == Dimension::Three {
            normal.sample(rng)
        } else {
            0.0
        };
        disp[i] = [vx, vy, vz];
        norm += vx * vx + vy * vy + vz * vz;
    }
    norm /= (dim.as_usize() as f64) * (points.len() as f64);
    norm = norm.sqrt();
    for i in 0..points.len() {
        points[i].x += STEP_SIZE * disp[i][0] / norm;
        points[i].y += STEP_SIZE * disp[i][1] / norm;
        points[i].z += STEP_SIZE * disp[i][2] / norm;
    }
}

/// Gradient of the total Lennard-Jones energy with respect to each
/// particle's position.
fn gradient(points: &[Point]) -> Vec<Point> {
    let mut grad = vec![
        Point {
            x: 0.0,
            y: 0.0,
            z: 0.0
        };
        points.len()
    ];
    for i in 0..points.len() {
        for j in (i + 1)..points.len() {
            let dx = points[i].x - points[j].x;
            let dy = points[i].y - points[j].y;
            let dz = points[i].z - points[j].z;
            let r = (dx * dx + dy * dy + dz * dz).sqrt();
            let r_star = SIGMA / r;
            let r_star2 = r_star * r_star;
            let r_star4 = r_star2 * r_star2;
            let r_star6 = r_star4 * r_star2;
            let r_star12 = r_star6 * r_star6;
            let dv_dr = 24.0 * EPSILON / r * (r_star6 - 2.0 * r_star12);
            let coeff = dv_dr / r;
            grad[i].x += coeff * dx;
            grad[i].y += coeff * dy;
            grad[i].z += coeff * dz;
            grad[j].x -= coeff * dx;
            grad[j].y -= coeff * dy;
            grad[j].z -= coeff * dz;
        }
    }
    grad
}

/// Moves every particle by `GD_LEARNING_RATE * gradient`, clipped to an
/// overall length of at most `STEP_SIZE` so a huge initial gradient (from
/// the near-overlapping starting configuration) can't blow the system
/// apart. Once the gradient is small enough that the learning-rate step no
/// longer needs clipping, steps shrink along with it, letting the system
/// actually converge instead of perpetually oscillating at a fixed step
/// length. Returns `(gnorm, gmax)`: the gradient's overall L2 norm and the
/// largest absolute component of any single particle's gradient, both
/// measured before the step is taken.
fn gradient_descent_step(points: &mut [Point]) -> (f64, f64) {
    let grad = gradient(points);
    let mut sum_sq = 0.0;
    let mut gmax: f64 = 0.0;
    for g in &grad {
        sum_sq += g.x * g.x + g.y * g.y + g.z * g.z;
        gmax = gmax.max(g.x.abs()).max(g.y.abs()).max(g.z.abs());
    }
    let gnorm = sum_sq.sqrt();
    let scale = if GD_LEARNING_RATE * gnorm > STEP_SIZE {
        STEP_SIZE / gnorm
    } else {
        GD_LEARNING_RATE
    };
    for (p, g) in points.iter_mut().zip(grad.iter()) {
        p.x -= scale * g.x;
        p.y -= scale * g.y;
        p.z -= scale * g.z;
    }
    (gnorm, gmax)
}

/// Formats a duration given in seconds as `HH:MM:SS`, or `n/a` if it isn't
/// finite.
fn format_duration(seconds: f64) -> String {
    if !seconds.is_finite() || seconds < 0.0 {
        return "n/a".to_string();
    }
    let total_secs = seconds.round() as u64;
    let h = total_secs / 3600;
    let m = (total_secs % 3600) / 60;
    let s = total_secs % 60;
    format!("{h:02}:{m:02}:{s:02}")
}

/// Per-iteration diagnostics specific to the algorithm in use, shown in the
/// progress log alongside the shared iteration/energy/throughput fields.
enum StepInfo {
    SimulatedAnnealing { temperature: f64 },
    GradientDescent { gnorm: f64, gmax: f64 },
}

impl StepInfo {
    fn log_str(&self) -> String {
        match self {
            StepInfo::SimulatedAnnealing { temperature } => format!("T={temperature:.6e}"),
            StepInfo::GradientDescent { gnorm, gmax } => {
                format!("gnorm={gnorm:.6e} | gmax={gmax:.6e}")
            }
        }
    }
}

fn write_xyz_frame<W: Write>(writer: &mut W, points: &[Point], comment: &str) -> IoResult<()> {
    writeln!(writer, "{}", points.len())?;
    writeln!(writer, "{comment}")?;
    for p in points {
        writeln!(writer, "Ar {} {} {}", p.x, p.y, p.z)?;
    }
    Ok(())
}

fn optimize(cli: &Cli) -> IoResult<()> {
    let seed = cli.seed.unwrap_or_else(|| rand::rng().random());
    log::info!("rng seed: {seed}");
    log::info!("algorithm: {}", cli.algorithm.as_str());
    let mut rng = StdRng::seed_from_u64(seed);
    let n_points = cli.n_points;
    let dim = cli.dim;

    let mut points = vec![
        Point {
            x: 0.0,
            y: 0.0,
            z: 0.0
        };
        n_points
    ];
    update(&mut rng, &mut points, dim);
    let mut current_energy = energy(&points);
    let mut best = current_energy;
    let mut best_points = points.clone();
    let mut best_iter: u64 = 0;

    let algo = cli.algorithm.as_str();
    let trajectory_path = cli.output_dir.join(format!(
        "trajectory_n{n_points}_{}d_{algo}.xyz",
        dim.as_usize()
    ));
    let mut trajectory = BufWriter::new(File::create(&trajectory_path)?);
    log::info!("writing trajectory frames to {}", trajectory_path.display());

    let start = Instant::now();
    for k in 0..cli.max_iters {
        let (step_info, converged) = match cli.algorithm {
            Algorithm::SimulatedAnnealing => {
                let temperature = 1.0 - ((k + 1) as f64) / (cli.max_iters as f64);
                let mut new_points = points.clone();
                update(&mut rng, &mut new_points, dim);
                let new_energy = energy(&new_points);

                if new_energy < best {
                    best = new_energy;
                    best_points = new_points.clone();
                    best_iter = k;
                }

                if new_energy < current_energy
                    || rng.random_range(0.0..1.0)
                        < (-(new_energy - current_energy) / temperature).exp()
                {
                    current_energy = new_energy;
                    points = new_points;
                }
                (StepInfo::SimulatedAnnealing { temperature }, false)
            }
            Algorithm::GradientDescent => {
                let (gnorm, gmax) = gradient_descent_step(&mut points);
                current_energy = energy(&points);
                if current_energy < best {
                    best = current_energy;
                    best_points = points.clone();
                    best_iter = k;
                }
                (
                    StepInfo::GradientDescent { gnorm, gmax },
                    gnorm < cli.gd_tol,
                )
            }
        };

        if k % cli.log_interval == 0 || converged {
            let elapsed = start.elapsed().as_secs_f64();
            let done = (k + 1) as f64;
            let throughput = done / elapsed.max(1e-9);
            let eta = format_duration(((cli.max_iters - k - 1) as f64) / throughput);

            log::info!(
                "iter={k:>10}/{max_iters} ({pct:5.1}%) | {step_str} | E={current_energy:.6e} | best={best:.6e} | throughput={throughput:.2e} it/s | ETA={eta}",
                max_iters = cli.max_iters,
                pct = 100.0 * (k as f64) / (cli.max_iters as f64),
                step_str = step_info.log_str(),
            );

            let comment = format!("iter={k} E={current_energy:.6e} best={best:.6e}");
            write_xyz_frame(&mut trajectory, &points, &comment)?;
        }

        if converged {
            log::info!("gradient norm below tolerance ({:.6e}); stopping early at iter {k}", cli.gd_tol);
            break;
        }
    }
    trajectory.flush()?;

    log::info!("best energy: {best:.6e} (iter {best_iter})");

    let cluster_path = cli
        .output_dir
        .join(format!("cluster_{n_points}_{algo}.xyz"));
    let mut file = File::create(&cluster_path)?;
    write_xyz_frame(&mut file, &best_points, "")?;
    log::info!("best cluster written to {}", cluster_path.display());

    Ok(())
}

fn main() -> IoResult<()> {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .format_timestamp_millis()
        .init();

    let cli = Cli::parse();
    std::fs::create_dir_all(&cli.output_dir)?;
    optimize(&cli)?;

    Ok(())
}
