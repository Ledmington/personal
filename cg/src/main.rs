use rand::{Rng, rng};

fn dot_unstable(a: &[f64], b: &[f64]) -> f64 {
    assert!(a.len() == b.len());
    let n = a.len();
    let mut s: f64 = 0.0;
    for i in 0..n {
        s += a[i] * b[i];
    }
    s
}

// Kahan sum
fn dot(a: &[f64], b: &[f64]) -> f64 {
    assert!(a.len() == b.len());
    let mut sum: f64 = 0.0;
    let mut c: f64 = 0.0;

    for i in 0..a.len() {
        let prod = a[i] * b[i];
        let y = prod - c;
        let t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }

    sum
}

fn norm_squared(v: &[f64]) -> f64 {
    dot(v, v)
}

fn conjugate_gradient(a: &[f64], b: &[f64], x: &mut [f64], max_iter: usize) {
    let n: usize = x.len();
    assert!(b.len() == n);
    assert!(a.len() == n * n);

    // Assumes x=0, so initialize r=b
    let mut r: Vec<f64> = vec![0.0; n];
    r.clone_from_slice(b);
    let mut p: Vec<f64> = r.clone();
    let mut k: usize = 0;
    while k < max_iter {
        // Compute A * p_k
        let mut ap: Vec<f64> = vec![0.0; n];
        for i in 0..n {
            // Borrow i-th row
            let a_row_i: &[f64] = &a[(i * n)..((i + 1) * n)];
            ap[i] = dot(a_row_i, &p);
            // for j in 0..n {
            //     ap[i] += a[i * n + j] * p[j];
            // }
        }
        // Compute p_k^T * A * p_k
        let pap: f64 = dot(&p, &ap);

        let r2 = norm_squared(&r);
        let alpha = r2 / pap;

        for i in 0..n {
            x[i] += alpha * p[i];
        }

        let mut r_next: Vec<f64> = vec![0.0; n];
        for i in 0..n {
            r_next[i] = r[i] - alpha * ap[i];
        }

        let norm2_r = norm_squared(&r_next);
        let norm_r = norm2_r.sqrt();
        println!(" {:3}: || r_k ||_2 = {:.6e}", k, norm_r);
        if norm_r < 1e-6 {
            println!("|| r_k ||_2 = {} < 1e-6. Exiting.", norm_r);
            break;
        }

        let beta: f64 = norm2_r / r2;

        for i in 0..n {
            p[i] = r_next[i] + beta * p[i];
        }

        // r = r_next.clone();
        std::mem::swap(&mut r, &mut r_next);

        k += 1;
    }
}

fn main() {
    let n: usize = 100;

    let mut rng = rng();

    let mut a: Vec<f64> = vec![0.0; n * n];
    let mut b: Vec<f64> = vec![0.0; n];

    for i in 0..n {
        b[i] = rng.random_range(-1.0..1.0);
        for j in 0..n {
            a[i * n + j] = rng.random_range(-1.0..1.0);
        }
    }

    let mut x: Vec<f64> = vec![0.0; n];

    let max_iter = 100;
    conjugate_gradient(&a, &b, &mut x, max_iter);

    println!("Solution:");
    for i in 0..n {
        println!("  x_{} = {:.6e}", i, x[i]);
    }
}
