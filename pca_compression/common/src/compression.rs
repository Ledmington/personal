use std::{
    fs::{self, File},
    io::{BufReader, Read, Write},
};

use crate::{
    conversion::{convert_vec_f64_to_vec_u8, convert_vec_u8_to_vec_f64},
    pca::{divisors, linearize, pca, project_data, split_into_chunks},
};

fn read_file_bytes(filename: &str) -> Vec<u8> {
    let input: File = File::open(filename).unwrap();
    let mut reader: BufReader<File> = BufReader::new(input);
    let mut buffer: Vec<u8> = Vec::new();

    // Read whole file into vector (FIXME?)
    reader.read_to_end(&mut buffer).unwrap();

    buffer
}

pub fn compress(input_filename: &str, output_filename: &str) {
    log::info!("Reading from '{}'", input_filename);

    let buffer: Vec<u8> = read_file_bytes(input_filename);
    let input_bytes: usize = buffer.len();

    log::info!("Read {} bytes", input_bytes);

    // Debug
    // print_buffer(&buffer);

    let mut pca_values: Vec<f64> = convert_vec_u8_to_vec_f64(&buffer);
    let n_pca_values: usize = pca_values.len();

    // scale all values from [0.0; 1.0] to [-1.0; 1.0]
    for pv in &mut pca_values {
        *pv = *pv * 2.0 - 1.0;
    }

    let possible_values = divisors(n_pca_values);
    log::debug!("Possible values: {:?}", possible_values);
    let k = *possible_values.iter().max().unwrap();

    log::debug!(
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
        for evj in explained_variance
            .iter_mut()
            .take(eigenvalues.len())
            .skip(i)
        {
            *evj += exp_var;
        }
    }

    // find the minimum number of principal components needed to have the "probability of one wrong bit" less than the number of input bits
    let mut minimum_principal_components: usize = 0;
    for i in 0..explained_variance.len() {
        let ev = explained_variance[i];
        let p = 1.0 / (1.0 - ev);
        log::debug!(
            "{} PCs: {} -> probability of 1 wrong bit = 1 in {}",
            i,
            explained_variance[i] * 100.0,
            1.0 / (1.0 - explained_variance[i])
        );
        if p.ceil() as usize >= input_bytes * 8 {
            // log::debug!(
            //     "{} PCs: {} -> probability of 1 wrong bit = 1 in {}",
            //     i,
            //     explained_variance[i] * 100.0,
            //     1.0 / (1.0 - explained_variance[i])
            // );
            if i < explained_variance.len() - 1 {
                log::debug!(
                    "{} PCs: {} -> probability of 1 wrong bit = 1 in {}",
                    i + 1,
                    explained_variance[i + 1] * 100.0,
                    1.0 / (1.0 - explained_variance[i + 1])
                );
            }
            minimum_principal_components = i;
            break;
        }
    }
    debug_assert!(minimum_principal_components > 0);
    debug_assert!(minimum_principal_components < explained_variance.len());

    let top_eigenvectors: Vec<Vec<f64>> = eigenvectors
        .iter()
        .take(minimum_principal_components)
        .cloned()
        .collect();
    let projected = project_data(&vectors, &top_eigenvectors);
    let mut linearized = linearize(&projected);

    // scale all values from [-1.0; 1.0] to [0.0; 1.0]
    for x in &mut linearized {
        debug_assert!((-1.0..1.0).contains(x));
        *x = *x / 2.0 + 1.0;
    }

    let output_bytes = convert_vec_f64_to_vec_u8(&linearized);
    log::info!("Writing compressed output to '{}'", output_filename);

    let mut file = fs::OpenOptions::new()
        .write(true)
        .create(true)
        .truncate(true)
        .open(output_filename)
        .unwrap();

    file.write_all(&output_bytes).unwrap();
}
