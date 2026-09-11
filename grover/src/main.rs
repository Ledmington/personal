#[derive(Clone, Copy, Debug)]
struct Complex {
    re: f64,
    im: f64,
}

impl Complex {
    fn real(v: f64) -> Complex {
        Complex { re: v, im: 0.0 }
    }
    fn norm_sq(self) -> f64 {
        self.re * self.re + self.im * self.im
    }
}

impl std::ops::Mul for Complex {
    type Output = Complex;
    fn mul(self, r: Complex) -> Complex {
        Complex {
            re: self.re * r.re - self.im * r.im,
            im: self.re * r.im + self.im * r.re,
        }
    }
}

impl std::ops::Add for Complex {
    type Output = Complex;
    fn add(self, r: Complex) -> Complex {
        Complex {
            re: self.re + r.re,
            im: self.im + r.im,
        }
    }
}

#[derive(Debug)]
struct Matrix {
    rows: usize,
    columns: usize,
    data: Vec<Complex>,
}

impl Matrix {
    fn one() -> Matrix {
        Matrix {
            rows: 1,
            columns: 1,
            data: vec![Complex::real(1.0)],
        }
    }

    fn ket_zero() -> Matrix {
        Matrix {
            rows: 2,
            columns: 1,
            data: vec![Complex::real(1.0), Complex::real(0.0)],
        }
    }

    fn ket_one() -> Matrix {
        Matrix {
            rows: 2,
            columns: 1,
            data: vec![Complex::real(0.0), Complex::real(1.0)],
        }
    }

    fn hadamard() -> Matrix {
        let h = 1.0 / 2.0_f64.sqrt();
        Matrix {
            rows: 2,
            columns: 2,
            data: vec![
                Complex::real(h),
                Complex::real(h),
                Complex::real(h),
                Complex::real(-h),
            ],
        }
    }

    fn identity() -> Matrix {
        Matrix {
            rows: 2,
            columns: 2,
            data: vec![
                Complex::real(1.0),
                Complex::real(0.0),
                Complex::real(0.0),
                Complex::real(1.0),
            ],
        }
    }

    fn pauli_x() -> Matrix {
        Matrix {
            rows: 2,
            columns: 2,
            data: vec![
                Complex::real(0.0),
                Complex::real(1.0),
                Complex::real(1.0),
                Complex::real(0.0),
            ],
        }
    }
}

fn tensor_product(a: &Matrix, b: &Matrix) -> Matrix {
    let nr = a.rows * b.rows;
    let nc = a.columns * b.columns;
    let mut d = vec![Complex::real(0.0); nr * nc];
    for i in 0..a.rows {
        for j in 0..b.rows {
            for k in 0..a.columns {
                for l in 0..b.columns {
                    d[(i * b.rows + j) * nc + k * b.columns + l] =
                        a.data[i * a.columns + k] * b.data[j * b.columns + l];
                }
            }
        }
    }
    Matrix {
        rows: nr,
        columns: nc,
        data: d,
    }
}

fn mat_vec_mul(m: &Matrix, v: &Matrix) -> Matrix {
    assert_eq!(m.columns, v.rows);
    let mut result = vec![Complex::real(0.0); m.rows];
    for i in 0..m.rows {
        for k in 0..m.columns {
            result[i] = result[i] + m.data[i * m.columns + k] * v.data[k];
        }
    }
    Matrix {
        rows: m.rows,
        columns: 1,
        data: result,
    }
}

fn mat_mul(a: &Matrix, b: &Matrix) -> Matrix {
    assert_eq!(a.columns, b.rows);
    let (n, p, q) = (a.rows, b.columns, a.columns);
    let mut d = vec![Complex::real(0.0); n * p];
    for i in 0..n {
        for j in 0..p {
            for k in 0..q {
                d[i * p + j] = d[i * p + j] + a.data[i * q + k] * b.data[k * p + j];
            }
        }
    }
    Matrix {
        rows: n,
        columns: p,
        data: d,
    }
}

/// Multi-controlled NOT: flips ancilla (last qubit) when all n data qubits are |1⟩.
fn build_multi_cnot(n: usize) -> Matrix {
    let dim = 1 << (n + 1);
    let mut d = vec![Complex::real(0.0); dim * dim];
    for i in 0..dim {
        d[i * dim + i] = Complex::real(1.0);
    }
    // data bits = (1<<n)-1, ancilla bit 0 or 1 -> indices all_ones*2 and all_ones*2+1
    let s0 = ((1 << n) - 1) * 2;
    let s1 = s0 + 1;
    d[s0 * dim + s0] = Complex::real(0.0);
    d[s1 * dim + s1] = Complex::real(0.0);
    d[s0 * dim + s1] = Complex::real(1.0);
    d[s1 * dim + s0] = Complex::real(1.0);
    Matrix {
        rows: dim,
        columns: dim,
        data: d,
    }
}

/// Phase-kickback oracle: U_f |x⟩|−⟩ = (−1)^f(x) |x⟩|−⟩
/// Built as NOT_solution * multi_cnot * NOT_solution.
fn build_oracle(n: usize, solution: usize) -> Matrix {
    // NOT layer: map solution to all_ones
    let mut flip = Matrix::one();
    for i in (0..n).rev() {
        let gate = if solution & (1 << i) != 0 {
            Matrix::identity()
        } else {
            Matrix::pauli_x()
        };
        flip = tensor_product(&flip, &gate);
    }
    flip = tensor_product(&flip, &Matrix::identity()); // identity on ancilla

    let mcnot = build_multi_cnot(n);
    let tmp = mat_mul(&mcnot, &flip);
    mat_mul(&flip, &tmp) // flip is its own inverse
}

/// Grover diffusion operator: 2|s⟩⟨s| - I, restricted to data qubits (X) I_ancilla.
/// Equivalent to H^n * (2|0⟩⟨0| - I) * H^n  on data register.
fn build_diffusion(n: usize) -> Matrix {
    let data_dim = 1 << n;

    // 2|0⟩⟨0| - I
    let mut d = vec![Complex::real(0.0); data_dim * data_dim];
    for i in 0..data_dim {
        for j in 0..data_dim {
            d[i * data_dim + j] = Complex::real(
                2.0 * (if i == 0 && j == 0 { 1.0 } else { 0.0 }) - (if i == j { 1.0 } else { 0.0 }),
            );
        }
    }
    let phase = Matrix {
        rows: data_dim,
        columns: data_dim,
        data: d,
    };

    let mut h_n = Matrix::one();
    for _ in 0..n {
        h_n = tensor_product(&h_n, &Matrix::hadamard());
    }

    // D_data = H^n * phase * H^n
    let tmp = mat_mul(&phase, &h_n);
    let d_data = mat_mul(&h_n, &tmp);

    tensor_product(&d_data, &Matrix::identity())
}

/// Return the most probable data-register state (marginalizing over ancilla).
fn measure(state: &Matrix, n: usize) -> usize {
    let data_dim = 1 << n;
    (0..data_dim)
        .max_by(|&a, &b| {
            let pa: f64 = (0..2).map(|anc| state.data[a * 2 + anc].norm_sq()).sum();
            let pb: f64 = (0..2).map(|anc| state.data[b * 2 + anc].norm_sq()).sum();
            pa.partial_cmp(&pb).unwrap()
        })
        .unwrap()
}

fn print_state(state: &Matrix, n: usize, label: &str) {
    println!("{}", label);
    let data_dim = 1 << n;
    for di in 0..data_dim {
        let prob: f64 = (0..2).map(|anc| state.data[di * 2 + anc].norm_sq()).sum();
        println!("  |{:0>width$b}⟩  prob = {:.4}", di, prob, width = n);
    }
}

fn main() {
    let n = 4; // Number of data qubits
    let big_n = 1 << n; // Search space size
    let solution = 2; // Target state
    assert!(solution < big_n);

    // Initial state: |0⟩^n (X) |1⟩
    let mut state = Matrix::one();
    for _ in 0..n {
        state = tensor_product(&state, &Matrix::ket_zero());
    }
    state = tensor_product(&state, &Matrix::ket_one());

    // H^(n+1): create uniform superposition
    let mut h_all = Matrix::one();
    for _ in 0..(n + 1) {
        h_all = tensor_product(&h_all, &Matrix::hadamard());
    }
    state = mat_vec_mul(&h_all, &state);

    // Build oracle and diffusion operator
    let oracle = build_oracle(n, solution);
    let diffusion = build_diffusion(n);

    // Optimal iteration count: floor(pi/4 * sqrt(N))  (minimum 1)
    let iterations = ((std::f64::consts::PI / 4.0) * (big_n as f64).sqrt()).floor() as usize;
    let iterations = iterations.max(1);

    println!(
        "N={}, target=|{:0>width$b}⟩={}, iterations={}\n",
        big_n,
        solution,
        solution,
        iterations,
        width = n
    );
    print_state(&state, n, "Initial (after H):");

    // Grover algorithm
    for iter in 0..iterations {
        state = mat_vec_mul(&oracle, &state); // phase-flip target
        state = mat_vec_mul(&diffusion, &state); // inversion about mean
        print_state(&state, n, &format!("\nAfter iteration {}:", iter + 1));
    }

    // Measure
    let result = measure(&state, n);
    println!(
        "\nMeasured: |{:0>width$b}⟩ = {}  (target was {})",
        result,
        result,
        solution,
        width = n
    );
    assert_eq!(result, solution, "Wrong answer!");
    println!("✓ Correct!");
}
