use plotters::prelude::*;
use rand::Rng;
use std::{
    cmp,
    collections::{HashSet, VecDeque},
    error::Error,
};

pub fn plot_dense_matrix(
    matrix: &[f64],
    n: usize,
    output_file: &str,
    width: u32,
    height: u32,
) -> Result<(), Box<dyn Error>> {
    let root = BitMapBackend::new(output_file, (width, height)).into_drawing_area();
    root.fill(&WHITE)?;

    // let max_val = matrix.iter().fold(f64::MIN, f64::max);
    // let min_val = matrix.iter().fold(f64::MAX, f64::min);

    let cell_width = width as f64 / n as f64;
    let cell_height = height as f64 / n as f64;

    for i in 0..n {
        for j in 0..n {
            if matrix[i * n + j] == 0.0 {
                continue;
            }

            let x0 = (j as f64 * cell_width) as i32;
            let y0 = (i as f64 * cell_height) as i32;
            let x1 = ((j + 1) as f64 * cell_width) as i32;
            let y1 = ((i + 1) as f64 * cell_height) as i32;

            root.draw(&Rectangle::new([(x0, y0), (x1, y1)], BLUE.filled()))?;
        }
    }

    root.present()?;
    Ok(())
}

fn cuthill_mckee(m: &mut [f64], n: usize) {
    // Convert to adjacency list
    let mut adj: Vec<Vec<usize>> = vec![Vec::new(); n];
    for i in 0..n {
        for j in 0..n {
            if m[i * n + j] == 0.0 {
                continue;
            }

            adj[i].push(j);
        }
    }

    let mut visited: Vec<bool> = vec![false; n];
    let mut ordering: Vec<usize> = Vec::with_capacity(n);

    for start in 0..n {
        if visited[start] {
            continue;
        }

        bfs(start, &adj, &mut visited, &mut ordering);
    }

    assert!(visited.iter().all(|x| *x));

    assert!(*ordering.iter().min().unwrap() == 0);
    assert!(*ordering.iter().max().unwrap() == n - 1);
    assert!(ordering.clone().into_iter().collect::<HashSet<_>>().len() == n);

    // Apply ordering by rows
    let mut tmp: Vec<f64> = vec![0.0; n * n];
    for i in 0..n {
        let j = ordering[i];

        // write in row i the old row j
        for k in 0..n {
            tmp[i * n + k] = m[j * n + k];
        }
    }

    // Apply ordering by columns
    let mut tmp2: Vec<f64> = vec![0.0; n * n];
    for i in 0..n {
        let j = ordering[i];

        // write in column i the old column j
        for k in 0..n {
            tmp2[k * n + i] = tmp[k * n + j];
        }
    }

    // Copy tmp into m
    m.copy_from_slice(&tmp2);
}

fn bfs(start: usize, adj: &[Vec<usize>], visited: &mut [bool], ordering: &mut Vec<usize>) {
    let mut queue = VecDeque::new();
    queue.push_back(start);

    while let Some(node) = queue.pop_front() {
        if visited[node] {
            continue;
        }

        visited[node] = true;
        ordering.push(node);

        for neighbor in adj[node].iter() {
            queue.push_back(*neighbor);
        }
    }
}

fn find_max_badwidth(m: &[f64], n: usize) -> usize {
    let mut max_bandwidth = 0;
    for i in 0..n {
        for j in (i + 1)..n {
            if m[i * n + j] == 0.0 {
                continue;
            }
            let bandwidth = j - i;
            max_bandwidth = cmp::max(bandwidth, max_bandwidth);
        }
    }
    max_bandwidth
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let n: usize = 1000;

    let mut rng = rand::rng();

    let mut m: Vec<f64> = vec![0.0; n * n];

    // Initialize diagonal
    for i in 0..n {
        m[i * n + i] = rng.random_range(-1.0..1.0);
    }

    let p: f64 = 0.005;
    assert!(p > 0.0 && p < 1.0);
    println!(
        "Generating a random symmetric matrix with sparsity {:.1}%.",
        p * 100.0
    );

    for i in 0..n {
        for j in (i + 1)..n {
            if rng.random_range(0.0..1.0) < p {
                m[i * n + j] = rng.random_range(-1.0..1.0);
                m[j * n + i] = rng.random_range(-1.0..1.0);
            }
        }
    }

    println!("Resulting bandwidth: {}.", find_max_badwidth(&m, n));

    let filename = "before.png";
    plot_dense_matrix(&m, n, filename, 600, 600)?;
    println!("Plot saved to {}.", filename);

    println!("Running Cuthill-McKee algorithm.");
    cuthill_mckee(&mut m, n);
    println!("Resulting bandwidth: {}.", find_max_badwidth(&m, n));

    let filename = "after.png";
    plot_dense_matrix(&m, n, filename, 600, 600)?;
    println!("Plot saved to {}.", filename);

    Ok(())
}
