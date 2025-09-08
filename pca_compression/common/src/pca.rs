// Compute column means
fn column_means(data: &[Vec<f64>]) -> Vec<f64> {
    let n_samples = data.len();
    let n_features = data[0].len();
    let mut means = vec![0.0; n_features];

    for row in data {
        for (j, &val) in row.iter().enumerate() {
            means[j] += val;
        }
    }
    for m in &mut means {
        *m /= n_samples as f64;
    }
    means
}

// Center the dataset
fn center_data(data: &[Vec<f64>], means: &Vec<f64>) -> Vec<Vec<f64>> {
    data.iter()
        .map(|row| row.iter().zip(means).map(|(x, m)| x - m).collect())
        .collect()
}

// Compute covariance matrix (features x features)
fn covariance_matrix(centered: &[Vec<f64>]) -> Vec<Vec<f64>> {
    let n_samples = centered.len();
    let n_features = centered[0].len();
    let mut cov = vec![vec![0.0; n_features]; n_features];

    for i in 0..n_features {
        for j in i..n_features {
            let mut sum = 0.0;
            for k in 0..n_samples {
                sum += centered[k][i] * centered[k][j];
            }
            let val = sum / (n_samples as f64 - 1.0);
            cov[i][j] = val;
            cov[j][i] = val; // symmetric
        }
    }
    cov
}

fn norm(v: &[f64]) -> f64 {
    debug_assert!(!v.is_empty());
    let mut s: f64 = 0.0;
    for i in 0..v.len() {
        s += v[i] * v[i];
    }
    s.sqrt()
}

fn normalize(v: &mut [f64]) {
    debug_assert!(!v.is_empty());
    let nv = norm(v);
    for i in 0..v.len() {
        v[i] /= nv;
    }
}

fn is_symmetric(matrix: &[Vec<f64>], difference: f64) -> bool {
    let rows = matrix.len();
    let cols = matrix[0].len();

    for i in 0..rows {
        for j in (i + 1)..cols {
            if f64::abs(matrix[i][j] - matrix[j][i]) > difference {
                return false;
            }
        }
    }

    true
}

fn power_iteration(matrix: &[Vec<f64>], n_iter: usize) -> (f64, Vec<f64>) {
    debug_assert!(!matrix.is_empty());
    debug_assert!(matrix.iter().all(|v| v.len() == matrix[0].len()));

    debug_assert!(is_symmetric(matrix, 1e-12));

    let n = matrix.len();
    let mut b = vec![1.0; n];
    normalize(&mut b); // is this necessary?

    for _ in 0..n_iter {
        let mut new_b = vec![0.0; n];
        for i in 0..n {
            for j in 0..n {
                new_b[i] += matrix[i][j] * b[j];
            }
        }
        b = new_b;
        normalize(&mut b);
    }

    // Rayleigh quotient for eigenvalue
    let mut num = 0.0;
    let mut den = 0.0;
    for i in 0..n {
        let mut row_sum = 0.0;
        for j in 0..n {
            row_sum += matrix[i][j] * b[j];
        }
        num += b[i] * row_sum;
        den += b[i] * b[i];
    }
    let eigenvalue = num / den;

    (eigenvalue, b)
}

pub fn pca(data: &[Vec<f64>], n_components: usize) -> (Vec<Vec<f64>>, Vec<f64>, Vec<Vec<f64>>) {
    debug_assert!(n_components > 0);
    debug_assert!(!data.is_empty());

    // let n_samples = data.len();
    let n_features = data[0].len();

    debug_assert!(data.iter().all(|v| v.len() == n_features));
    debug_assert!(n_components <= n_features);

    let means = column_means(data);
    let centered = center_data(data, &means);
    let cov = covariance_matrix(&centered);

    let mut eigenvalues = Vec::new();
    let mut eigenvectors = Vec::new();
    let mut cov_copy = cov.clone();

    for i in 0..n_components {
        log::debug!("PCA iteration {} / {}", i, n_components);
        let (eigval, eigvec) = power_iteration(&cov_copy, 1000);
        eigenvalues.push(eigval);
        eigenvectors.push(eigvec.clone());

        // Deflate covariance: cov = cov - λ * v v^T
        for i in 0..cov_copy.len() {
            for j in 0..cov_copy.len() {
                cov_copy[i][j] -= eigval * eigvec[i] * eigvec[j];
            }
        }
    }

    // Project data
    let mut projected = vec![vec![0.0; n_components]; centered.len()];
    for (i, row) in centered.iter().enumerate() {
        for (c, eigvec) in eigenvectors.iter().enumerate() {
            projected[i][c] = row.iter().zip(eigvec).map(|(x, v)| x * v).sum();
        }
    }

    (projected, eigenvalues, eigenvectors)
}

pub fn split_into_chunks(arr: &[f64], k: usize) -> Vec<Vec<f64>> {
    debug_assert!(k > 0, "k must be > 0");
    debug_assert!(arr.len() % k == 0, "array length must be divisible by k");

    let chunk_size = arr.len() / k;
    let mut result: Vec<Vec<f64>> = Vec::new();
    let mut idx = 0;
    for i in 0..k {
        result.push(Vec::new());
        for _ in 0..chunk_size {
            result[i].push(arr[idx]);
            idx += 1;
        }
    }
    result
}

pub fn linearize(arr: &[Vec<f64>]) -> Vec<f64> {
    let mut v: Vec<f64> = Vec::new();
    for x in arr {
        for xi in x {
            v.push(*xi);
        }
    }
    v
}

pub fn divisors(x: usize) -> Vec<usize> {
    let mut tmp = Vec::new();
    for i in 2..((x as f64).sqrt().ceil() as usize + 1) {
        if x % i == 0 {
            tmp.push(i);
        }
    }
    debug_assert!(!tmp.is_empty(), "{} is a prime number: TODO.", x);
    tmp
}

pub fn project_data(centered: &Vec<Vec<f64>>, eigenvectors: &Vec<Vec<f64>>) -> Vec<Vec<f64>> {
    let n_samples = centered.len();
    let k = eigenvectors.len(); // number of components
    let n_features = centered[0].len();

    let mut projected = vec![vec![0.0; k]; n_samples];

    for i in 0..n_samples {
        for (comp_idx, eigvec) in eigenvectors.iter().enumerate() {
            projected[i][comp_idx] = (0..n_features).map(|j| centered[i][j] * eigvec[j]).sum();
        }
    }

    projected
}

#[cfg(test)]
mod tests {

    use super::*;
    use rand::{Rng, SeedableRng, rngs::StdRng};

    #[test]
    fn normalization() {
        let mut seed = [0u8; 32];
        rand::rng().fill(&mut seed);
        let mut rng: StdRng = StdRng::from_seed(seed);

        const N: usize = 1000;
        const D: usize = 10;
        for _ in 0..N {
            let mut v: Vec<f64> = (0..D).map(|_| rng.random_range(0.0..1.0)).collect();
            normalize(&mut v);
            assert!(f64::abs(norm(&v) - 1.0) <= 1e-12);
        }
    }

    #[test]
    fn symmetric_matrix_check() {
        let m = vec![
            vec![1.0, 0.0, 0.0],
            vec![0.0, 1.0, 0.0],
            vec![0.0, 0.0, 1.0],
        ];
        assert!(is_symmetric(&m, 0.0));
    }

    #[test]
    fn asymmetric_matrix_check() {
        let m = vec![
            vec![1.0, 0.0, 1e-12],
            vec![0.0, 1.0, 0.0],
            vec![0.0, 0.0, 1.0],
        ];
        assert!(is_symmetric(&m, 1e-12));
        assert!(!is_symmetric(&m, 1e-13));
    }
}
