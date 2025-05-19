use std::collections::HashMap;

use plotters::prelude::*;
use rand::Rng;
use rand_distr::Normal;

fn plot_clusters(
    filename: &str,
    points: &Vec<(f64, f64)>,
    labels: &Vec<usize>,
) -> Result<(), Box<dyn std::error::Error>> {
    assert!(points.len() == labels.len());

    println!("Saving plot on {}", filename);

    let root = BitMapBackend::new(filename, (800, 800)).into_drawing_area();
    root.fill(&WHITE)?;

    let (x_min, x_max) = points.iter().fold(
        (f64::INFINITY, f64::NEG_INFINITY),
        |(min_x, max_x), (x, _)| (min_x.min(*x), max_x.max(*x)),
    );
    let (y_min, y_max) = points.iter().fold(
        (f64::INFINITY, f64::NEG_INFINITY),
        |(min_y, max_y), (_, y)| (min_y.min(*y), max_y.max(*y)),
    );

    let mut chart = ChartBuilder::on(&root)
        .caption("Cluster Plot", ("sans-serif", 30))
        .margin(20)
        .x_label_area_size(40)
        .y_label_area_size(40)
        .build_cartesian_2d(x_min..x_max, y_min..y_max)?;

    chart.configure_mesh().draw()?;

    let colors = vec![
        &RED,
        &BLUE,
        &GREEN,
        &CYAN,
        &MAGENTA,
        &YELLOW,
        &BLACK,
        &RGBColor(255, 127, 0), // orange
        &RGBColor(127, 0, 255), // purple
        &RGBColor(0, 128, 128), // teal
    ];

    for (i, p) in points.iter().enumerate() {
        let color = colors[labels[i]];
        chart.draw_series(std::iter::once(Circle::new((p.0, p.1), 4, color.filled())))?;
    }

    Ok(())
}

fn squared_distance(a: &(f64, f64), b: &(f64, f64)) -> f64 {
    (a.0 - b.0).powi(2) + (a.1 - b.1).powi(2)
}

fn distance(a: &(f64, f64), b: &(f64, f64)) -> f64 {
    squared_distance(a, b).sqrt()
}

fn get_parent(union_find: &mut HashMap<usize, usize>, x: usize) -> usize {
    let p = union_find.get(&x).unwrap();
    if *p == x {
        return x;
    }
    let pp = get_parent(union_find, *p);
    union_find.insert(x, pp);
    pp
}

fn kruskal_clustering(points: &Vec<(f64, f64)>, k: usize, labels: &mut Vec<usize>) {
    assert!(k > 0);

    let n = points.len();
    assert!(n == labels.len());

    let mut union_find: HashMap<usize, usize> = HashMap::new();
    for i in 0..n {
        union_find.insert(i, i);
    }

    let mut edges: Vec<(usize, usize, f64)> = Vec::with_capacity(n * (n - 1) / 2);
    for i in 0..n {
        for j in (i + 1)..n {
            edges.push((i, j, distance(&points[i], &points[j])));
        }
    }
    edges.sort_by(|a, b| a.2.partial_cmp(&b.2).unwrap());

    let mut edge_idx: usize = 0;
    for _ in 0..(n - k) {
        let mut i: usize;
        let mut j: usize;
        loop {
            (i, j, _) = edges[edge_idx];

            if get_parent(&mut union_find, i) == get_parent(&mut union_find, j) {
                // skip
                edge_idx += 1;
                continue;
            } else {
                break;
            }
        }

        let pi = get_parent(&mut union_find, i);
        let pj = get_parent(&mut union_find, j);
        union_find.insert(pi, pj);
    }

    // assert!(
    //     k == union_find
    //         .clone()
    //         .into_values()
    //         .into_iter()
    //         .collect::<HashSet<_>>()
    //         .len()
    // );

    let mut lbl: HashMap<usize, usize> = HashMap::new();
    {
        let mut idx = 0;
        for i in 0..n {
            let pi = get_parent(&mut union_find, i);
            if !lbl.contains_key(&pi) {
                lbl.insert(pi, idx);
                idx += 1;
            }
        }
        assert!(idx == k);
    }

    for i in 0..n {
        labels[i] = *lbl.get(&get_parent(&mut union_find, i)).unwrap();
    }
}

fn compute_centroids(data: &Vec<(f64, f64)>, labels: &[usize], k: usize) -> Vec<(f64, f64)> {
    let mut centroids = vec![(0.0, 0.0); k];
    let mut counts = vec![0; k];

    for (point, &label) in data.iter().zip(labels.iter()) {
        centroids[label].0 += point.0;
        centroids[label].1 += point.1;

        counts[label] += 1;
    }

    for i in 0..k {
        if counts[i] > 0 {
            centroids[i].0 /= counts[i] as f64;
            centroids[i].1 /= counts[i] as f64;
        }
    }

    centroids
}

fn wcss(data: &Vec<(f64, f64)>, labels: &[usize], k: usize) -> f64 {
    let centroids = compute_centroids(data, labels, k);
    data.iter()
        .zip(labels.iter())
        .map(|(point, &label)| squared_distance(point, &centroids[label]))
        .sum()
}

fn plot_elbow(
    filename: &str,
    k_vals: &[usize],
    wcss_vals: &[f64],
) -> Result<(), Box<dyn std::error::Error>> {
    println!("Saving plot on {}", filename);

    let root = BitMapBackend::new(filename, (800, 600)).into_drawing_area();
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

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut rng = rand::rng();

    let n_clusters = 5;
    let n_points = 1_000;

    let mut points: Vec<(f64, f64)> = vec![(0.0, 0.0); n_points];
    let mut solution: Vec<usize> = vec![0; n_points];
    {
        let mut distributions = Vec::with_capacity(n_clusters);
        for _ in 0..n_clusters {
            distributions.push((
                Normal::new(rng.random_range(-10.0..10.0), rng.random_range(0.01..2.0)).unwrap(),
                Normal::new(rng.random_range(-10.0..10.0), rng.random_range(0.01..2.0)).unwrap(),
            ));
        }

        for i in 0..n_points {
            solution[i] = rng.random_range(0..n_clusters);
            points[i] = (
                rng.sample(distributions[solution[i]].0),
                rng.sample(distributions[solution[i]].1),
            );
        }
    }

    plot_clusters("input.png", &points, &solution)?;
    println!();

    let mut wcss_values = Vec::new();
    let k_vals: Vec<usize> = (2..(2 * n_clusters)).collect();
    for k in &k_vals {
        println!("Trying with {} clusters", k);
        let mut labels: Vec<usize> = vec![0; n_points];
        kruskal_clustering(&points, *k, &mut labels);
        let wcss = wcss(&points, &labels, *k);
        wcss_values.push(wcss);
        println!("WCSS = {}", wcss);
        plot_clusters(&format!("kruskal_{}.png", k), &points, &labels)?;
        println!();
    }

    plot_elbow("elbow.png", &k_vals, &wcss_values)?;

    Ok(())
}
