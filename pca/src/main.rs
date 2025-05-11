use plotters::prelude::*;

fn add_vertices(points: &mut Vec<Vec<f64>>, d: usize, size: f64) {
    for i in 0..(1 << d) {
        let mut vertex = Vec::with_capacity(d);
        for j in 0..d {
            if (i >> j) & 1 == 1 {
                vertex.push(size);
            } else {
                vertex.push(-size);
            }
        }
        points.push(vertex);
    }
}

fn add_edges(points: &mut Vec<Vec<f64>>, d: usize, step: f64, points_per_size: usize) {
    // Assumes the vector 'points' already contains the vertices of the cube
    assert!(points.len() == 1 << d);

    let vertices = points.clone();

    // For each edge (vertices differing in one coordinate), interpolate points
    for i in 0..vertices.len() {
        for j in (i + 1)..vertices.len() {
            let (v1, v2) = (&vertices[i], &vertices[j]);
            let differing_indices: Vec<_> = v1
                .iter()
                .zip(v2)
                .enumerate()
                .filter(|(_, (a, b))| (*a - *b).abs() > 1e-6)
                .map(|(idx, _)| idx)
                .collect();

            if differing_indices.len() == 1 {
                let idx = differing_indices[0];
                let mut step_vec = v1.clone();

                for k in 0..=points_per_size {
                    step_vec[idx] = v1[idx] + step * (k as f64);
                    points.push(step_vec.clone());
                }
            }
        }
    }
}

fn center(data: &mut [Vec<f64>]) {
    let n = data.len();
    let d = data[0].len();
    let mut m: Vec<f64> = vec![0.0; d];
    for di in data.iter().take(n) {
        for (j, mj) in m.iter_mut().enumerate().take(d) {
            *mj += di[j];
        }
    }
    for mi in m.iter_mut().take(d) {
        *mi /= n as f64;
    }
    for di in data.iter_mut().take(n) {
        for (j, mj) in m.iter().enumerate().take(d) {
            di[j] -= mj;
        }
    }
}

fn compute_covariance_matrix(data: &[Vec<f64>]) -> Vec<Vec<f64>> {
    let n_samples = data.len();
    let n_features = data[0].len();
    let mut cov = vec![vec![0.0; n_features]; n_features];

    for i in 0..n_features {
        for j in 0..n_features {
            let mut sum = 0.0;
            for row in data.iter() {
                sum += row[i] * row[j];
            }
            cov[i][j] = sum / (n_samples as f64 - 1.0);
        }
    }

    cov
}

fn dot(u: &[f64], v: &[f64]) -> f64 {
    u.iter().zip(v).map(|(a, b)| a * b).sum()
}

fn norm(v: &[f64]) -> f64 {
    dot(v, v).sqrt()
}

fn normalize(v: &mut [f64]) {
    let n = norm(v);
    for x in v.iter_mut() {
        *x /= n;
    }
}

fn matrix_vector_mul(matrix: &[Vec<f64>], vector: &[f64]) -> Vec<f64> {
    matrix.iter().map(|row| dot(row, vector)).collect()
}

fn power_iteration(matrix: &[Vec<f64>], max_iter: usize, tol: f64) -> Vec<f64> {
    let n = matrix.len();
    let mut v = vec![1.0; n];
    normalize(&mut v);

    for _ in 0..max_iter {
        let mv = matrix_vector_mul(matrix, &v);
        let mut new_v = mv.clone();
        normalize(&mut new_v);

        let diff = v
            .iter()
            .zip(&new_v)
            .map(|(a, b)| (a - b).abs())
            .fold(0.0, f64::max);
        v = new_v;
        if diff < tol {
            break;
        }
    }

    v
}

fn deflate(matrix: &mut [Vec<f64>], eigenvector: &[f64], eigenvalue: f64) {
    let n = matrix.len();

    for i in 0..n {
        for j in 0..n {
            matrix[i][j] -= eigenvalue * eigenvector[i] * eigenvector[j];
        }
    }
}

fn estimate_eigenvalue(matrix: &[Vec<f64>], eigenvector: &[f64]) -> f64 {
    let mv = matrix_vector_mul(matrix, eigenvector);
    dot(&mv, eigenvector)
}

fn project_to_2d(data: &[Vec<f64>]) -> Vec<(f64, f64)> {
    let mut data = data.to_owned();
    center(&mut data);

    let mut cov = compute_covariance_matrix(&data);

    let mut components: Vec<(f64, f64)> = Vec::new();

    let eigvec1 = power_iteration(&cov, 1000, 1e-8);
    let eigval1 = estimate_eigenvalue(&cov, &eigvec1);
    deflate(&mut cov, &eigvec1, eigval1);
    components.push((eigvec1[0], eigvec1[1]));

    let eigvec2 = power_iteration(&cov, 1000, 1e-8);
    let eigval2 = estimate_eigenvalue(&cov, &eigvec2);
    deflate(&mut cov, &eigvec2, eigval2);
    components.push((eigvec2[0], eigvec2[1]));

    data.iter()
        .map(|row| (dot(row, &eigvec1), dot(row, &eigvec2)))
        .collect()
}

fn plot(
    points: &[(f64, f64)],
    title: &str,
    filename: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    assert!(!points.is_empty());

    let root = BitMapBackend::new(filename, (800, 600)).into_drawing_area();
    root.fill(&WHITE)?;

    let (min_x, max_x) = points
        .iter()
        .map(|p| p.0)
        .fold((f64::INFINITY, f64::NEG_INFINITY), |(min, max), x| {
            (min.min(x), max.max(x))
        });
    let (min_y, max_y) = points
        .iter()
        .map(|p| p.1)
        .fold((f64::INFINITY, f64::NEG_INFINITY), |(min, max), y| {
            (min.min(y), max.max(y))
        });

    let mut chart = ChartBuilder::on(&root)
        .caption(title, ("sans-serif", 30))
        .margin(20)
        .x_label_area_size(30)
        .y_label_area_size(30)
        .build_cartesian_2d(min_x..max_x, min_y..max_y)?;

    chart.configure_mesh().draw()?;

    chart.draw_series(points.iter().map(|p| Circle::new(*p, 2, RED.filled())))?;

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let size: f64 = 1.0;
    let points_per_side: usize = 50;
    let step: f64 = (size * 2.0) / (points_per_side as f64);

    for dimensions in 3..10 {
        println!(
            "Generating wireframe of a {}-dimensional cube with {} points per side...",
            dimensions, points_per_side
        );
        let total_points: usize = (1 << dimensions) + points_per_side * (1 << dimensions);
        println!("Will generate {} total points.", total_points);
        println!("Will need {} total bytes.", total_points * 8);

        let mut data: Vec<Vec<f64>> = Vec::with_capacity(total_points);

        add_vertices(&mut data, dimensions, size);
        add_edges(&mut data, dimensions, step, points_per_side);

        let projected: Vec<(f64, f64)> = project_to_2d(&data);

        let filename = format!("square_{}d.png", dimensions);
        println!("Saving plot to {}", filename);
        plot(
            &projected,
            format!("{}D to 2D", dimensions).as_str(),
            &filename,
        )?;
        println!();
    }

    Ok(())
}
