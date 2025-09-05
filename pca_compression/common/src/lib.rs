use bitbuffer::{BigEndian, BitReadBuffer, BitReadStream, BitWriteStream};

const MASK: u64 = 0x001f_ffff_ffff_ffffu64;
// TODO: convert MASK_BITS to usize
const MASK_BITS: u32 = MASK.count_ones(); // 53

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

fn is_symmetric(matrix: &[Vec<f64>]) -> bool {
    let rows = matrix.len();
    let cols = matrix[0].len();

    for i in 0..rows {
        for j in (i + 1)..cols {
            if matrix[i][j] != matrix[j][i] {
                return false;
            }
        }
    }

    true
}

// Power iteration to find top eigenvector of a symmetric matrix
fn power_iteration(matrix: &[Vec<f64>], n_iter: usize) -> (f64, Vec<f64>) {
    assert!(!matrix.is_empty());
    assert!(matrix.iter().all(|v| v.len() == matrix[0].len()));
    assert!(is_symmetric(matrix));

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
    assert!(n_components > 0);
    assert!(!data.is_empty());
    assert!(data.iter().all(|v| v.len() == n_components));

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

pub fn print_buffer(buffer: &[u8]) -> String {
    const MAX_BYTES_PER_ROW: usize = 16;
    let mut s: String = "".to_owned();
    for i in 0..buffer.len() {
        if i % MAX_BYTES_PER_ROW == 0 {
            s.push_str(format!("{:08x}:", i).as_str());
        }
        s.push_str(format!(" {:02x}", buffer[i]).as_str());
        if i % MAX_BYTES_PER_ROW == MAX_BYTES_PER_ROW - 1 {
            s.push('\n');
        }
    }
    s.push('\n');
    s
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

fn convert_u64_to_f64(input: u64) -> f64 {
    assert!(input & (!MASK) == 0); // shouldn't this be an if with an error?
    let d: f64 = (input as f64) / ((1u64 << MASK_BITS) as f64);
    assert!(!d.is_nan());
    assert!(d.is_finite());
    assert!(d >= 0.0 && d < 1.0);
    return d;
}

fn convert_f64_to_u64(input: f64) -> u64 {
    // shouldn't these assertions be an if with an error?
    assert!(!input.is_nan());
    assert!(input.is_finite());
    assert!(input >= 0.0 && input < 1.0);
    let x: u64 = (input * ((1u64 << MASK_BITS) as f64)) as u64;
    assert!(x & (!MASK) == 0);
    return x;
}

pub fn convert_vec_u8_to_vec_f64(input: &[u8]) -> Vec<f64> {
    let n_input_bytes: usize = input.len();
    let n_input_bits: usize = n_input_bytes * 8;
    let n_output_elements: usize = (n_input_bits + (MASK_BITS - 1) as usize) / MASK_BITS as usize;

    println!(
        "{} bytes -> {} values for PCA",
        n_input_bytes, n_output_elements
    );

    let mut output_elements: Vec<f64> = vec![0f64; n_output_elements];

    let input_buffer = BitReadBuffer::new(&input, BigEndian);
    let mut stream = BitReadStream::new(input_buffer);
    let mut output_element_index: usize = 0;

    let mut tmp: u64 = 0;
    for bit_index in 0..n_input_bits {
        // add each bit one by one
        if stream.read_bool().unwrap() {
            tmp |= 1u64 << ((MASK_BITS as usize) - 1 - bit_index);
        }

        if bit_index % (MASK_BITS as usize) == 0 {
            // time to write!
            let d: f64 = convert_u64_to_f64(tmp);
            output_elements[output_element_index] = d;
            output_element_index += 1;
            tmp = 0;
        }
    }

    assert!(output_element_index == n_output_elements);

    output_elements
}

pub fn convert_vec_f64_to_vec_u8(input: &[f64], n_bits: usize) -> Vec<u8> {
    let n_input_elements: usize = input.len();
    let n_input_bytes: usize = n_input_elements * 8;
    let n_input_bits: usize = n_input_bytes * 8;
    let n_output_bytes: usize = ((n_input_elements * (MASK_BITS as usize)) + 7) / 8;

    println!(
        "{} values from PCA ({} bits) -> {} bytes",
        n_input_elements, n_bits, n_output_bytes
    );

    let mut output_bytes: Vec<u8> = vec![0u8; n_output_bytes];

    let input_as_u8: Vec<u8> = input.iter().flat_map(|&d| d.to_be_bytes()).collect();
    let input_buffer = BitReadBuffer::new(&input_as_u8, BigEndian);
    let mut input_stream = BitReadStream::new(input_buffer);
    let mut output_stream = BitWriteStream::new(&mut output_bytes, BigEndian);

    for bit_index in 0..n_input_bits {
        // skip bits outside the mask
        if (bit_index % 64) < (64 - (MASK_BITS as usize)) {
            continue;
        }

        output_stream
            .write_bool(input_stream.read_bool().unwrap())
            .unwrap();
    }

    output_bytes
}

#[cfg(test)]
mod tests {

    use super::*;
    use rand::{
        Rng, RngCore, SeedableRng,
        rngs::{StdRng, ThreadRng},
    };
    use rstest::rstest;

    #[rstest]
    #[case(0u64, 0f64)]
    #[case(0x001f_ffff_ffff_ffffu64, 0.9999999999999999f64)]
    #[case(0x000f_ffff_ffff_ffffu64, 0.4999999999999999f64)]
    fn u64_to_f64_conversion(#[case] x: u64, #[case] d: f64) {
        assert_eq!(d, convert_u64_to_f64(x));
    }

    #[rstest]
    #[case(0f64, 0u64)]
    #[case(0.9999999999999999f64, 0x001f_ffff_ffff_ffffu64)]
    #[case(0.4999999999999999f64, 0x000f_ffff_ffff_ffffu64)]
    fn f64_to_u64_conversion(#[case] d: f64, #[case] x: u64) {
        assert_eq!(x, convert_f64_to_u64(d));
    }

    #[test]
    fn random_u64_to_f64_conversion() {
        let mut seed = [0u8; 32];
        rand::rng().fill(&mut seed);
        let mut rng: StdRng = StdRng::from_seed(seed);

        let n: usize = 1_000_000;
        for _ in 0..n {
            let x: u64 = rng.next_u64() & MASK;
            let converted: u64 = convert_f64_to_u64(convert_u64_to_f64(x));
            assert_eq!(x, converted, "Seed : {:?}", seed);
        }
    }

    #[test]
    fn random_f64_to_u64_conversion() {
        let mut seed = [0u8; 32];
        rand::rng().fill(&mut seed);
        let mut rng: StdRng = StdRng::from_seed(seed);

        let n: usize = 1_000_000;
        for _ in 0..n {
            let d: f64 = rng.random_range(0.0..1.0);
            let tmp = convert_f64_to_u64(d);
            let converted: f64 = convert_u64_to_f64(tmp);
            assert_eq!(d, converted, "Seed : {:?}", seed);
        }
    }

    #[rstest]
    #[case(1)]
    #[case(2)]
    #[case(4)]
    #[case(8)]
    // #[case(16)]
    // #[case(32)]
    // #[case(64)]
    // #[case(128)]
    // #[case(256)]
    // #[case(512)]
    // #[case(1024)]
    fn byte_vec_conversion(#[case] n: usize) {
        let mut rng: ThreadRng = rand::rng();
        let mut input: Vec<u8> = vec![0u8; n];
        for i in 0..n {
            input[i] = rng.random_range(0..256) as u8;
        }
        let d: Vec<f64> = convert_vec_u8_to_vec_f64(&input);
        for x in d.iter() {
            println!("{:016x}", f64::to_bits(*x));
        }
        let output: Vec<u8> = convert_vec_f64_to_vec_u8(&d, n * 8);
        assert_eq!(
            input,
            output,
            "\nINPUT:\n{}\nOUTPUT:\n{}\n",
            print_buffer(&input),
            print_buffer(&output)
        );
    }
}
