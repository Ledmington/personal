use core::f64;

use plotters::prelude::*;
use rand::Rng;
use rand_distr::{Distribution, Normal};

const A: f64 = 1.0;
const B: f64 = 100.0;

fn rosenbrock(v: &[f64]) -> f64 {
    let n = v.len();
    let mut s: f64 = 0.0;
    for i in 0..(n - 1) {
        let x: f64 = v[i];
        let y: f64 = v[i + 1];
        let t1: f64 = A - x;
        let t2: f64 = y - x * x;
        s += B * t2 * t2 + t1 * t1;
    }
    s
}

fn p(s: f64, s_new: f64, t: f64) -> f64 {
    if s_new < s {
        return 1.0;
    }
    (-(s_new - s) / t).exp()
}

fn lerp(a: f64, b: f64, t: f64) -> f64 {
    a * (1.0 - t) + b * t
}

fn plot_function_with_points(
    output_path: &str,
    x_range: (f64, f64),
    points: &[(f64, f64)],
) -> Result<(), Box<dyn std::error::Error>> {
    let root = BitMapBackend::new(output_path, (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;

    let mut chart = ChartBuilder::on(&root)
        .caption("Simulated Annealing on Rosenbrock", ("sans-serif", 30))
        .margin(20)
        .x_label_area_size(40)
        .y_label_area_size(40)
        .build_cartesian_2d(x_range.0..x_range.1, x_range.0..x_range.1)?;

    chart.configure_mesh().draw()?;

    // Plotting individual points with increasing opacity
    let total_points = points.len().max(1); // avoid division by zero
    let min_alpha = 0.1;
    let max_alpha = 1.0;
    chart.draw_series(points.iter().enumerate().map(|(i, &(x, y))| {
        // Linearly increasing alpha from 0.1 to 1.0
        let alpha = lerp(min_alpha, max_alpha, i as f64 / (total_points - 1) as f64);
        let color = RGBAColor(0, 0, 255, alpha); // Red with increasing alpha
        Circle::new((x, y), 5, color.filled())
    }))?;

    // Drawing a red cross at the function's known minimum
    let min_x = A;
    let min_y = A * A;
    let cross_size = 10;
    chart.draw_series(std::iter::once(PathElement::new(
        vec![
            (min_x - cross_size as f64 * 0.01, min_y),
            (min_x + cross_size as f64 * 0.01, min_y),
        ],
        RED,
    )))?;
    chart.draw_series(std::iter::once(PathElement::new(
        vec![
            (min_x, min_y + cross_size as f64 * 0.01),
            (min_x, min_y - cross_size as f64 * 0.01),
        ],
        RED,
    )))?;

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut rng = rand::rng();
    let normal = Normal::new(0.0, 1.0).unwrap();
    let d: usize = 2;
    let attempts: usize = 1000;
    let lower_bound: f64 = -3.0;
    let upper_bound: f64 = 3.0;

    assert!(lower_bound < upper_bound);

    // check that the minimum of Rosenbrock is inside
    assert!(A.powi(d as i32) >= lower_bound && A.powi(d as i32) <= upper_bound);

    let mut points: Vec<(f64, f64)> = vec![(0.0, 0.0); attempts];

    let mut best: Vec<f64> = vec![0.0; d];
    let mut fbest: f64 = rosenbrock(&best);

    let mut current: Vec<f64> = vec![0.0; d];
    let mut fcurrent: f64;

    for k in 0..attempts {
        let temperature = 1.0 - ((k as f64) / ((attempts - 1) as f64));

        let mut m: f64 = 0.0;
        for ci in current.iter_mut().take(d) {
            let x = normal.sample(&mut rng);
            *ci = x;
            m += x * x;
        }
        m = m.sqrt();
        let r: f64 = rng.random_range(0.0..1.0);
        for i in 0..d {
            current[i] = (best[i] + current[i] * r / m).clamp(lower_bound, upper_bound);
        }

        fcurrent = rosenbrock(&current);

        if p(fbest, fcurrent, temperature) >= rng.random_range(0.0..1.0) {
            fbest = fcurrent;
            best[..d].copy_from_slice(&current[..d]);
            print!("f({:+.3}", best[0]);
            for bi in best.iter().take(d).skip(1) {
                print!("; {:+.3}", bi);
            }
            println!(") = {:+.6}", fbest);

            if d == 2 {
                points[k] = (best[0], best[1]);
            }
        }
    }

    plot_function_with_points("output.png", (lower_bound, upper_bound), &points)?;
    println!("Plot saved to output.png");

    Ok(())
}
