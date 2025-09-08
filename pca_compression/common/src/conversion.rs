use bitbuffer::{BigEndian, BitReadBuffer, BitReadStream, BitWriteStream};

const MASK: u64 = 0x001f_ffff_ffff_ffffu64;
const MASK_BITS: usize = MASK.count_ones() as usize; // 53

pub fn print_buffer(buffer: &[u8]) -> String {
    const MAX_BYTES_PER_ROW: usize = 16;
    let mut s: String = "".to_owned();
    for (i, x) in buffer.iter().enumerate() {
        if i % MAX_BYTES_PER_ROW == 0 {
            s.push_str(format!("{:08x}:", i).as_str());
        }
        s.push_str(format!(" {:02x}", x).as_str());
        if i % MAX_BYTES_PER_ROW == MAX_BYTES_PER_ROW - 1 {
            s.push('\n');
        }
    }
    s.push('\n');
    s
}

fn convert_u64_to_f64(input: u64) -> f64 {
    debug_assert_eq!(input & (!MASK), 0, "Invalid input: 0x{:016x}", input);
    let d = (input as f64) / ((1u64 << MASK_BITS) as f64);
    debug_assert!(
        d.is_finite(),
        "Converted f64 is not finite: {:016x} -> {:.20}",
        input,
        d
    );
    debug_assert!(
        (0.0..1.0).contains(&d),
        "Converted f64 is not in range: {:016x} -> {:.20}",
        input,
        d
    );
    d
}

fn convert_f64_to_u64(input: f64) -> u64 {
    debug_assert!(input.is_finite(), "Input f64 is not finite: {:.20}", input);
    debug_assert!(
        (0.0..1.0).contains(&input),
        "Input f64 is not in range: {:.20}",
        input
    );
    let x: u64 = (input * ((1u64 << MASK_BITS) as f64)) as u64;
    debug_assert_eq!(
        x & (!MASK),
        0,
        "Invalid converted u64: {:.20} -> 0x{:016x}",
        input,
        x
    );
    x
}

pub fn convert_vec_u8_to_vec_f64(input: &[u8]) -> Vec<f64> {
    let n_input_bits: usize = input.len() * 8;
    let n_output_elements: usize = n_input_bits.div_ceil(MASK_BITS);

    let input_buffer = BitReadBuffer::new(input, BigEndian);
    let mut stream = BitReadStream::new(input_buffer);

    let mut output_elements = Vec::with_capacity(n_output_elements);

    let mut tmp: u64 = 0;
    let mut bit_in_chunk: usize = 0;

    for _ in 0..n_input_bits {
        let bit = stream.read_bool().unwrap();
        if bit {
            let shift = (MASK_BITS - 1) - bit_in_chunk;
            tmp |= 1u64 << shift;
        }
        bit_in_chunk += 1;

        if bit_in_chunk == MASK_BITS {
            output_elements.push(convert_u64_to_f64(tmp));
            tmp = 0;
            bit_in_chunk = 0;
        }
    }

    if bit_in_chunk > 0 {
        output_elements.push(convert_u64_to_f64(tmp));
    }

    output_elements
}

pub fn convert_vec_f64_to_vec_u8(input: &[f64]) -> Vec<u8> {
    convert_vec_f64_bits_to_vec_u8(input, input.len() * MASK_BITS)
}

fn convert_vec_f64_bits_to_vec_u8(input: &[f64], n_bits: usize) -> Vec<u8> {
    debug_assert!(input.len() * 64 >= n_bits);

    // Output size is determined by the number of original bits, not by 53 * elements.
    let n_output_bytes: usize = n_bits.div_ceil(8);

    log::debug!(
        "{} values from PCA ({} bits) -> {} bytes",
        input.len(),
        n_bits,
        n_output_bytes
    );

    let mut output_bytes: Vec<u8> = vec![0u8; n_output_bytes];
    let mut output_stream = BitWriteStream::new(&mut output_bytes, BigEndian);

    let mut remaining = n_bits;

    for &d in input {
        if remaining == 0 {
            break;
        }

        let x: u64 = convert_f64_to_u64(d);
        // We only need the top `take` bits from this 53-bit chunk
        let bits_to_write = remaining.min(MASK_BITS);

        // Write bits [52 .. 52-(take-1)] i.e., from top down
        for i in 0..bits_to_write {
            let mask: u64 = 1u64 << (MASK_BITS - 1 - i);
            let bit: bool = (x & mask) != 0;
            output_stream.write_bool(bit).unwrap();
        }

        remaining -= bits_to_write;
    }

    // By construction, we should have written exactly n_bits
    assert_eq!(remaining, 0);
    assert_eq!(output_stream.bit_len(), n_bits);
    assert_eq!(output_stream.byte_len(), n_output_bytes);

    // there are always some spurious 0x00 bytes at the beginning
    let skip = output_bytes.len() - n_output_bytes;
    output_bytes.into_iter().skip(skip).collect()
}

#[cfg(test)]
mod tests {

    use super::*;
    use rand::{Rng, RngCore, SeedableRng, rngs::StdRng};
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
    #[case(16)]
    #[case(32)]
    #[case(64)]
    #[case(128)]
    #[case(256)]
    #[case(512)]
    #[case(1024)]
    fn byte_vec_conversion(#[case] n: usize) {
        let mut seed = [0u8; 32];
        rand::rng().fill(&mut seed);
        let mut rng: StdRng = StdRng::from_seed(seed);

        let mut input: Vec<u8> = vec![0u8; n];
        for i in 0..n {
            input[i] = rng.random_range(0..256) as u8;
        }
        let d: Vec<f64> = convert_vec_u8_to_vec_f64(&input);
        let output: Vec<u8> = convert_vec_f64_bits_to_vec_u8(&d, n * 8);
        assert_eq!(
            input,
            output,
            "Seed: {:?}\n\nINPUT:\n{}\nOUTPUT:\n{}\n",
            seed,
            print_buffer(&input),
            print_buffer(&output)
        );
    }
}
