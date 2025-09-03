use std::{env, fs, io::Write};

use common::{
    convert_f64_to_u8, convert_u8_to_f64, divisors, linearize, pca, project_data, read_file_bytes,
    split_into_chunks,
};

fn main() {
    if false {
        let data = vec![
            vec![2.5, 2.4, 1.0],
            vec![0.5, 0.7, 1.2],
            vec![2.2, 2.9, 0.9],
            vec![1.9, 2.2, 1.1],
            vec![3.1, 3.0, 0.95],
        ];

        let (projected, eigenvalues, eigenvectors) = pca(&data, 2);

        println!("Eigenvalues: {:?}", eigenvalues);
        println!("Eigenvectors: {:?}", eigenvectors);
        println!("Projected Data: {:?}", projected);
    }

    let args: Vec<String> = env::args().collect();
    let filename = &args[1];

    println!("Reading from '{}'", filename);

    let buffer: Vec<u8> = read_file_bytes(filename);
    let input_bytes: usize = buffer.len();

    println!("Read {} bytes", input_bytes);

    // Debug
    // print_buffer(&buffer);

    let mut pca_values: Vec<f64> = convert_u8_to_f64(&buffer);
    let n_pca_values: usize = pca_values.len();

    // scale all values from [0.0; 1.0] to [-1.0; 1.0]
    for i in 0..pca_values.len() {
        pca_values[i] = pca_values[i] * 2.0 - 1.0;
    }

    println!();
    let possible_values = divisors(n_pca_values);
    let k = *possible_values.iter().max().unwrap();

    println!(
        "Trying with {} vectors of {} dimensions each",
        k,
        n_pca_values / k
    );
    let vectors: Vec<Vec<f64>> = split_into_chunks(&pca_values, k);
    let (_projected, eigenvalues, eigenvectors) = pca(&vectors, k);

    let total_variance: f64 = eigenvalues.iter().sum();
    let mut explained_variance: Vec<f64> = vec![0.0; eigenvalues.len()];
    for i in 0..eigenvalues.len() {
        let exp_var = eigenvalues[i] / total_variance;
        for j in i..eigenvalues.len() {
            explained_variance[j] += exp_var;
        }
    }

    // find the minimum number of principal components needed to have the "probability of one wrong bit" less than the number of input bits
    let mut minimum_principal_components: usize = 0;
    for i in 0..explained_variance.len() {
        let ev = explained_variance[i];
        let p = 1.0 / (1.0 - ev);
        if p.ceil() as usize >= input_bytes * 8 {
            println!(
                "{}: {} -> probability of 1 wrong bit = 1 in {}",
                i,
                explained_variance[i] * 100.0,
                1.0 / (1.0 - explained_variance[i])
            );
            println!(
                "{}: {} -> probability of 1 wrong bit = 1 in {}",
                i + 1,
                explained_variance[i + 1] * 100.0,
                1.0 / (1.0 - explained_variance[i + 1])
            );
            minimum_principal_components = i;
            break;
        }
    }
    assert!(
        minimum_principal_components > 0 && minimum_principal_components < explained_variance.len()
    );

    let top_eigenvectors: Vec<Vec<f64>> = eigenvectors
        .iter()
        .take(minimum_principal_components)
        .cloned()
        .collect();
    let projected = project_data(&vectors, &top_eigenvectors);
    let linearized = linearize(&projected);

    let output_bytes = convert_f64_to_u8(&linearized);
    let output_filename = filename.to_owned() + ".pcazip";
    println!("Writing compressed output to '{}'", output_filename);

    let mut file = fs::OpenOptions::new()
        .write(true)
        .create(true)
        .truncate(true)
        .open(output_filename)
        .unwrap();

    file.write_all(&output_bytes).unwrap();
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = 2 + 2;
        assert_eq!(result, 4);
    }
}
