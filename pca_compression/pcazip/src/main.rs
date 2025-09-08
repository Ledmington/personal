use common::compression::compress;
use env_logger::Env;
use std::{env, path::Path};

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: pcazip <file_to_compress>");
        return;
    }

    let input_filename = &args[1];

    if !Path::new(input_filename).exists() {
        eprintln!("File '{}' does not exist.", input_filename);
        return;
    }

    let output_filename = input_filename.to_owned() + ".pcazip";

    env_logger::Builder::from_env(Env::default().default_filter_or("debug")).init();

    compress(input_filename, &output_filename);
}
