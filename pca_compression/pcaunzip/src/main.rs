fn reconstruct_data(
    projected: &Vec<Vec<f64>>,    // Z, n_samples × k
    eigenvectors: &Vec<Vec<f64>>, // V, k × n_features
    means: &Vec<f64>,             // μ, n_features
) -> Vec<Vec<f64>> {
    let n_samples = projected.len();
    let n_features = means.len();
    let k = eigenvectors.len();

    let mut reconstructed = vec![vec![0.0; n_features]; n_samples];

    for i in 0..n_samples {
        for j in 0..n_features {
            // sum_k Z[i][k] * V[k][j]
            let mut val = 0.0;
            for l in 0..k {
                val += projected[i][l] * eigenvectors[l][j];
            }
            reconstructed[i][j] = val + means[j];
        }
    }

    reconstructed
}

fn main() {
    /*
        PCA decomposition:

    𝑋_centered = 𝑍 * 𝑉^𝑇

    Where:
    𝑋_centered ∈ 𝑅^{𝑛×𝑚} is the mean-centered data.

    𝑉 ∈ 𝑅^{𝑚×𝑘} contains the eigenvectors (principal components).

    𝑍 ∈ 𝑅^{𝑛×𝑘} is the projected data (scores).

    From this formula:

    𝑋 = 𝑍 * 𝑉^𝑇 + 𝜇

    where 𝜇 is the column mean vector of the original data.
     */
    println!("Hello world!");
}
