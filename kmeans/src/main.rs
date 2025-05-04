use plotters::prelude::*;
use rand::Rng;
use rand_distr::Normal;
use std::f64;

fn euclidean_distance(a: &[f64], b: &[f64]) -> f64 {
    assert!(!a.is_empty());
    assert!(a.len() == b.len());
    let mut s: f64 = 0.0;
    for i in 0..a.len() {
        let d = a[i] - b[i];
        s += d * d;
    }
    s.sqrt()
}

#[allow(clippy::needless_range_loop)]
fn kmeans(x: &[Vec<f64>], max_iter: usize, k: usize) -> (Vec<usize>, Vec<Vec<f64>>, usize) {
    assert!(!x.is_empty());
    assert!(!x[0].is_empty());
    assert!(k >= 1);

    let d: usize = x[0].len();

    let mut centroids: Vec<Vec<f64>> = vec![vec![0.0; d]; k];
    let mut labels: Vec<usize> = vec![0; x.len()];

    // Initialize clusters "randomly"
    for i in 0..x.len() {
        labels[i] = i % k;
    }

    let mut i: usize = 0;
    while i < max_iter {
        // Compute centroids
        {
            let mut sums: Vec<Vec<f64>> = vec![vec![0.0; d]; k];
            let mut counts: Vec<usize> = vec![0; k];
            for i in 0..x.len() {
                let label = labels[i];
                for j in 0..d {
                    sums[label][j] += x[i][j];
                }
                counts[label] += 1;
            }
            for i in 0..k {
                for j in 0..d {
                    centroids[i][j] = sums[i][j] / (counts[i] as f64);
                }
            }
        }

        // Compute distances and update labels
        let mut at_least_one_label_changed: bool = false;
        for i in 0..x.len() {
            let mut min_dist = f64::MAX;
            let mut min_label = 0;
            for j in 0..k {
                let dist = euclidean_distance(&x[i], &centroids[j]);
                if dist < min_dist {
                    min_dist = dist;
                    min_label = j;
                }
            }
            if labels[i] != min_label {
                at_least_one_label_changed = true;
            }
            labels[i] = min_label;
        }

        if !at_least_one_label_changed {
            break;
        }

        i += 1
    }

    (labels, centroids, i)
}

fn plot_elbow(k_vals: &[usize], wcss_vals: &[f64]) -> Result<(), Box<dyn std::error::Error>> {
    let root = BitMapBackend::new("elbow.png", (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;

    let max_wcss = wcss_vals.iter().cloned().fold(f64::MIN, f64::max);

    let mut chart = ChartBuilder::on(&root)
        .caption("Elbow Plot", ("sans-serif", 40))
        .margin(40)
        .x_label_area_size(40)
        .y_label_area_size(50)
        .build_cartesian_2d(
            *k_vals.first().unwrap() as f64..*k_vals.last().unwrap() as f64,
            0.0..max_wcss,
        )?;

    chart.configure_mesh().x_desc("k").y_desc("WCSS").draw()?;

    chart
        .draw_series(LineSeries::new(
            k_vals
                .iter()
                .zip(wcss_vals.iter())
                .map(|(&k, &wcss)| (k as f64, wcss)),
            &RED,
        ))?
        .label("WCSS")
        .legend(|(x, y)| PathElement::new([(x, y), (x + 20, y)], RED));

    chart.configure_series_labels().border_style(BLACK).draw()?;

    Ok(())
}

fn plot_clusters(
    data: &[Vec<f64>],
    labels: &[usize],
    centroids: &[Vec<f64>],
    filename: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    if data[0].len() != 2 {
        println!("Skipping plot: data is not 2D");
        return Ok(());
    }

    let root = BitMapBackend::new(filename, (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;

    let x_range = data.iter().map(|p| p[0]).fold(f64::INFINITY, f64::min)
        ..data.iter().map(|p| p[0]).fold(f64::NEG_INFINITY, f64::max);
    let y_range = data.iter().map(|p| p[1]).fold(f64::INFINITY, f64::min)
        ..data.iter().map(|p| p[1]).fold(f64::NEG_INFINITY, f64::max);

    let mut chart = ChartBuilder::on(&root)
        .caption("Input clusters", ("sans-serif", 30))
        .margin(20)
        .x_label_area_size(40)
        .y_label_area_size(40)
        .build_cartesian_2d(x_range.clone(), y_range.clone())?;

    chart.configure_mesh().x_desc("X").y_desc("Y").draw()?;

    let colors = &[&RED, &BLUE, &GREEN, &MAGENTA, &CYAN, &BLACK, &YELLOW];

    for i in 0..data.len() {
        let point = &data[i];
        chart.draw_series(std::iter::once(Circle::new(
            (point[0], point[1]),
            1,
            colors[labels[i] % colors.len()].filled(),
        )))?;
    }

    // Draw centroids as larger black circles
    for centroid in centroids {
        chart.draw_series(std::iter::once(Circle::new(
            (centroid[0], centroid[1]),
            8,
            BLACK.filled(),
        )))?;
    }

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let num_samples: usize = 100_000;
    let dimensions: usize = 2;
    let clusters: usize = 7;

    println!(
        "Generating {} clusters of {} points total in {} dimensions.",
        clusters, num_samples, dimensions
    );
    println!();

    let mut rng = rand::rng();

    // Generate random means and std.dev. for clusters
    let mut means: Vec<Vec<f64>> = vec![vec![0.0; dimensions]; clusters];
    let mut stddevs: Vec<f64> = vec![0.0; clusters];
    for i in 0..clusters {
        stddevs[i] = rng.random_range(-10.0..10.0);
        for j in 0..dimensions {
            means[i][j] = rng.random_range(-10.0..10.0);
        }
    }

    let mut x: Vec<Vec<f64>> = vec![vec![0.0; dimensions]; num_samples];
    let mut y: Vec<usize> = vec![0; num_samples];
    for i in 0..x.len() {
        let cluster_idx: usize = rng.random_range(0..clusters);
        y[i] = cluster_idx;
        for j in 0..dimensions {
            x[i][j] = rng.sample(Normal::new(means[cluster_idx][j], stddevs[cluster_idx]).unwrap());
        }
    }

    // Compute centroids
    let mut centroids: Vec<Vec<f64>> = vec![vec![0.0; dimensions]; clusters];
    {
        let mut counts: Vec<usize> = vec![0; clusters];
        for i in 0..num_samples {
            let cluster_idx: usize = y[i];
            for j in 0..dimensions {
                centroids[cluster_idx][j] += x[i][j];
            }
            counts[cluster_idx] += 1;
        }
        for i in 0..clusters {
            for j in 0..dimensions {
                centroids[i][j] /= counts[i] as f64;
            }
        }
    }

    if dimensions == 2 {
        println!("Plotting dataset.");
        plot_clusters(&x, &y, &centroids, "input.png")?;
    } else {
        println!("Skipping dataset plot because data is not 2D.");
    }

    let range = 2..=(2 * clusters);
    let mut k_vals = Vec::new();
    let mut wcss_vals = Vec::new();
    for k in range {
        let max_iter = 1000;
        let (labels, centroids, actual_max_iterations) = kmeans(&x, max_iter, k);

        // Compute WCSS
        let mut wcss = 0.0;
        for i in 0..num_samples {
            let centroid_idx = labels[i];
            let centroid = &centroids[centroid_idx];
            let sample = &x[i];
            wcss += euclidean_distance(sample, centroid);
        }
        println!(
            " {:2} -> max_iter: {:3} | wcss: {:.6}",
            k, actual_max_iterations, wcss
        );

        k_vals.push(k);
        wcss_vals.push(wcss);
    }

    println!();
    println!("Saving elbow plot.");
    plot_elbow(&k_vals, &wcss_vals)?;
    println!("Elbow plot saved as 'elbow.png'");

    println!();
    // Finding best k by looking for value of k with highest (approximated) second derivative
    let mut highest_deriv = -f64::MAX;
    let mut best_k = 0;
    for i in 1..(wcss_vals.len() - 1) {
        let deriv = wcss_vals[i - 1] - 2.0 * wcss_vals[i] + wcss_vals[i + 1];
        if deriv > highest_deriv {
            highest_deriv = deriv;
            best_k = k_vals[i];
        }
    }
    println!(
        "It seems that the correct value of clusters was {}.",
        best_k
    );
    println!();

    Ok(())
}
