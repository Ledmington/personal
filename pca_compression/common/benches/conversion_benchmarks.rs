use criterion::{Criterion, criterion_group, criterion_main};
use rand::{Rng, SeedableRng, rngs::StdRng};

fn bench_conversion(c: &mut Criterion) {
    c.bench_function("convert vec<u8> to vec<f64>", |b| {
        let mut seed = [0u8; 32];
        rand::rng().fill(&mut seed);
        let mut rng: StdRng = StdRng::from_seed(seed);

        const N: usize = 1000;
        let mut input: Vec<u8> = vec![0u8; N];
        for i in 0..N {
            input[i] = rng.random_range(0..256) as u8;
        }

        b.iter(|| common::conversion::convert_vec_u8_to_vec_f64(&input));
    });
}

criterion_group!(benches, bench_conversion);
criterion_main!(benches);
