use plotters::prelude::*;
use rand::Rng;
use rand_distr::Normal;

fn func(x: &[f64], beta: &[f64]) -> f64 {
    let n = x.len();
    assert!(beta.len() == n + 1);
    let mut s = beta[0];
    for i in 0..n {
        s += x[i] * beta[i + 1];
    }
    s
}

fn plot(
    x: &[f64],
    y: &[f64],
    slope: f64,
    intercept: f64,
    filename: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    assert!(x.len() == y.len());

    let x_min = x.iter().cloned().reduce(f64::min).unwrap() - 1.0;
    let x_max = x.iter().cloned().reduce(f64::max).unwrap() + 1.0;
    let y_min = y.iter().cloned().reduce(f64::min).unwrap() - 1.0;
    let y_max = y.iter().cloned().reduce(f64::max).unwrap() + 1.0;

    let root = BitMapBackend::new(filename, (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;
    let mut chart = ChartBuilder::on(&root)
        .caption("Linear Regression Plot", ("sans-serif", 30))
        .margin(40)
        .x_label_area_size(40)
        .y_label_area_size(40)
        .build_cartesian_2d(x_min..x_max, y_min..y_max)?;

    chart.configure_mesh().draw()?;

    chart.draw_series((0..(x.len())).map(|i| Circle::new((x[i], y[i]), 4, RED.filled())))?;

    let line = [
        (x_min, intercept + slope * x_min),
        (x_max, intercept + slope * x_max),
    ];
    chart.draw_series(LineSeries::new(line, &BLUE))?;

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut rng = rand::rng();

    let n: usize = 1000;
    let dimensions: usize = 1;

    println!("Generating {} points in {}D space...", n, dimensions);
    let mut solution: Vec<f64> = vec![0.0; dimensions + 1];
    for si in solution.iter_mut().take(dimensions + 1) {
        *si = rng.random_range(-3.0..3.0);
    }
    print!("Solution: y =");
    for (i, si) in solution.iter().enumerate().take(dimensions + 1) {
        print!(" {:+.3}", *si);
        if i > 0 {
            print!("*x_{}", i - 1);
        }
    }
    println!();

    let mut x: Vec<Vec<f64>> = vec![vec![0.0; dimensions]; n];
    let mut y: Vec<f64> = vec![0.0; n];

    let noise_distr = Normal::new(0.0, 4.0).unwrap();
    for i in 0..n {
        for j in 0..dimensions {
            x[i][j] = rng.random_range(-10.0..10.0);
        }
        y[i] = func(&x[i], &solution) + rng.sample(noise_distr);
    }

    let mut x_mean: Vec<f64> = vec![0.0; dimensions];
    let mut y_mean: f64 = 0.0;
    for i in 0..n {
        y_mean += y[i];
        for (j, xmj) in x_mean.iter_mut().enumerate().take(dimensions) {
            *xmj += x[i][j];
        }
    }
    y_mean /= n as f64;
    for xmj in x_mean.iter_mut().take(dimensions) {
        *xmj /= n as f64;
    }

    let mut sxy: Vec<f64> = vec![0.0; dimensions];
    let mut sxx: Vec<f64> = vec![0.0; dimensions];
    for i in 0..n {
        for j in 0..dimensions {
            let dx = x[i][j] - x_mean[j];
            sxy[j] += dx * (y[i] - y_mean);
            sxx[j] += dx * dx;
        }
    }

    let mut beta: Vec<f64> = vec![0.0; dimensions];
    for i in 0..dimensions {
        beta[i] = sxy[i] / sxx[i];
    }
    let mut intercept: f64 = 0.0;
    for i in 0..n {
        let mut b = beta.clone();
        b.insert(0, 0.0);
        intercept += y[i] - func(&x[i], &b);
    }
    intercept /= n as f64;
    beta.insert(0, intercept);

    print!("Computed: y =");
    for (i, bi) in beta.iter().enumerate().take(dimensions + 1) {
        print!(" {:+.3}", *bi);
        if i > 0 {
            print!("*x_{}", i - 1);
        }
    }
    println!();

    println!(
        "RMSE: {:.6e}",
        ((0..n)
            .map(|i| (y[i] - func(&x[i], &beta)).powi(2))
            .sum::<f64>()
            / (n as f64))
            .sqrt()
    );

    if dimensions == 1 {
        let mut x2: Vec<f64> = vec![0.0; n];
        for i in 0..n {
            x2[i] = x[i][0];
        }
        let filename = "regression.png";
        plot(&x2, &y, beta[0], intercept, filename)?;
        println!("Saved plot at {}", filename);
    }

    Ok(())
}
