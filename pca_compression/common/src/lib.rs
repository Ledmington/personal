use std::{
    fs::File,
    io::{BufReader, Read},
};

use bitbuffer::{BigEndian, BitReadBuffer, BitReadStream, BitWriteStream};

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

// Normalize a vector
fn normalize(v: &mut [f64]) {
    assert!(!v.is_empty());
    let mut s: f64 = 0.0;
    for i in 0..v.len() {
        s += v[i] * v[i];
    }
    s = s.sqrt();
    for i in 0..v.len() {
        v[i] /= s;
    }
}

// Power iteration to find top eigenvector of a symmetric matrix
fn power_iteration(matrix: &[Vec<f64>], n_iter: usize) -> (f64, Vec<f64>) {
    let n = matrix.len();
    let mut b = vec![1.0; n];
    normalize(&mut b);

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

// PCA: compute top n_components
pub fn pca(data: &[Vec<f64>], n_components: usize) -> (Vec<Vec<f64>>, Vec<f64>, Vec<Vec<f64>>) {
    let means = column_means(data);
    let centered = center_data(data, &means);
    let cov = covariance_matrix(&centered);

    let mut eigenvalues = Vec::new();
    let mut eigenvectors = Vec::new();
    let mut cov_copy = cov.clone();

    for _ in 0..n_components {
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

pub fn print_buffer(buffer: &[u8]) {
    const MAX_BYTES_PER_ROW: usize = 16;
    for i in 0..buffer.len() {
        if i % MAX_BYTES_PER_ROW == 0 {
            print!("{:08x}:", i);
        }
        print!(" {:02x}", buffer[i]);
        if i % MAX_BYTES_PER_ROW == MAX_BYTES_PER_ROW - 1 {
            println!();
        }
    }
    println!();
}

pub fn split_into_chunks(arr: &[f64], k: usize) -> Vec<Vec<f64>> {
    assert!(k > 0, "k must be > 0");
    assert!(arr.len() % k == 0, "array length must be divisible by k");

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
    tmp
}

pub fn read_file_bytes(filename: &str) -> Vec<u8> {
    let input: File = File::open(filename).unwrap();
    let mut reader: BufReader<File> = BufReader::new(input);
    let mut buffer: Vec<u8> = Vec::new();

    // Read whole file into vector (FIXME?)
    reader.read_to_end(&mut buffer).unwrap();

    buffer
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

pub fn convert_u8_to_f64(input: &[u8]) -> Vec<f64> {
    // read 61 bits from input stream to create a "random" double precision floating point value in the [0;1] range.
    const MASK: u64 = 0x3fef_ffff_ffff_ffff;
    assert!(MASK.count_ones() == 61);

    let input_bytes: usize = input.len();

    let n_pca_values: usize = ((input_bytes * 8) as f64 / 61.0).ceil() as usize;

    println!("{} bytes -> {} values for PCA", input_bytes, n_pca_values);

    let mut pca_values: Vec<f64> = vec![0.0; n_pca_values];

    let bit_buffer = BitReadBuffer::new(&input, BigEndian);
    let mut stream = BitReadStream::new(bit_buffer);
    let mut idx: usize = 0;
    for _ in 0..(n_pca_values - 1) {
        // 0x3fef_ffff_ffff_ffff
        let mut tmp: u64;
        // first, read 9 consecutive bits
        tmp = stream.read_int::<u64>(9).unwrap() << 53;
        // then, read another 52 consecutive bits
        tmp |= stream.read_int::<u64>(52).unwrap();
        assert!(tmp.count_ones() <= 61);

        let x = f64::from_bits(tmp);
        assert!(x >= 0.0 && x <= 1.0);
        pca_values[idx] = x;
        idx += 1;
    }
    // the last value is padded
    {
        let mut tmp: u64;
        // first, read 9 consecutive bits
        tmp = stream.read_int::<u64>(9).unwrap() << 53;
        // then, read another 52 consecutive bits
        tmp |= stream
            .read_int::<u64>((input_bytes * 8) - stream.pos())
            .unwrap();
        assert!(tmp.count_ones() <= 61);

        let x = f64::from_bits(tmp);
        assert!(x >= 0.0 && x <= 1.0);
        pca_values[idx] = x;
    }

    pca_values
}

pub fn convert_f64_to_u8(input: &[f64]) -> Vec<u8> {
    // use the mask to extract the only relevant 61 bits and encode those
    const MASK: u64 = 0x3fef_ffff_ffff_ffff;
    assert!(MASK.count_ones() == 61);

    let input_elements: usize = input.len();

    let n_output_bytes: usize = (((input_elements * 64) as f64 / 61.0).ceil() as usize + 7) / 8;

    println!(
        "{} values from PCA -> {} bytes",
        input_elements, n_output_bytes
    );

    let mut output_bytes: Vec<u8> = vec![0u8; n_output_bytes];

    let mut stream = BitWriteStream::new(&mut output_bytes, BigEndian);

    for i in 0..input.len() {
        let d: f64 = input[i];
        let x: u64 = f64::to_bits(d) & MASK;

        // write first 9 bits
        stream
            .write_int((x & 0x3fe0_0000_0000_0000u64) >> 53, 9)
            .unwrap();

        // write other 52 bits
        stream.write_int(x & 0x000f_ffff_ffff_ffffu64, 52).unwrap();
    }

    output_bytes
}
