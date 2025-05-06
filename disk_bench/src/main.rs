use itertools::Itertools;
use rand::Rng;

enum Direction {
    Left,
    Right,
}

struct Batch {
    num_cylinders: u32,
    initial_head_position: u32,
    direction: Direction,
    requests: Vec<u32>,
}

fn fcfs(batch: &Batch) -> u32 {
    let mut total = 0;
    let mut current = batch.initial_head_position;
    for r in batch.requests.iter() {
        total += (current as i32 - *r as i32).unsigned_abs();
        current = *r;
    }
    total
}

fn sstf(batch: &Batch) -> u32 {
    let mut remaining: Vec<u32> = batch.requests.to_vec();
    let mut total = 0;
    let mut current = batch.initial_head_position;

    while !remaining.is_empty() {
        let (i, &closest) = remaining
            .iter()
            .enumerate()
            .min_by_key(|&(_, &r)| (r as i32 - current as i32).abs())
            .unwrap();
        total += (closest as i32 - current as i32).unsigned_abs();
        current = closest;
        remaining.remove(i);
    }

    total
}

fn scan(batch: &Batch) -> u32 {
    let mut reqs = batch.requests.to_vec();
    reqs.sort();
    let mut total = 0;
    let mut current = batch.initial_head_position;
    let max_cyl = batch.num_cylinders - 1;

    let (left, right): (Vec<u32>, Vec<u32>) =
        reqs.iter().partition(|&&r| r < batch.initial_head_position);

    match batch.direction {
        Direction::Right => {
            for &r in right.iter() {
                total += (r as i32 - current as i32).unsigned_abs();
                current = r;
            }
            if current != max_cyl {
                total += max_cyl - current;
                current = max_cyl;
            }
            for &r in left.iter().rev() {
                total += (current as i32 - r as i32).unsigned_abs();
                current = r;
            }
        }
        Direction::Left => {
            for &r in left.iter().rev() {
                total += (current as i32 - r as i32).unsigned_abs();
                current = r;
            }
            if current != 0 {
                total += current;
                current = 0;
            }
            for &r in right.iter() {
                total += (r as i32 - current as i32).unsigned_abs();
                current = r;
            }
        }
    }

    total
}

fn c_scan(batch: &Batch) -> u32 {
    let mut reqs = batch.requests.to_vec();
    reqs.sort();
    let mut total = 0;
    let mut current = batch.initial_head_position;
    let max_cyl = batch.num_cylinders - 1;

    let right: Vec<u32> = reqs
        .iter()
        .cloned()
        .filter(|&r| r >= batch.initial_head_position)
        .collect();
    let left: Vec<u32> = reqs
        .iter()
        .cloned()
        .filter(|&r| r < batch.initial_head_position)
        .collect();

    for r in &right {
        total += (*r as i32 - current as i32).unsigned_abs();
        current = *r;
    }

    if current != max_cyl {
        total += max_cyl - current;
    }

    total += max_cyl; // from max to 0
    current = 0;

    for r in &left {
        total += (*r as i32 - current as i32).unsigned_abs();
        current = *r;
    }

    total
}

fn look(batch: &Batch) -> u32 {
    let mut reqs = batch.requests.to_vec();
    reqs.sort();
    let mut total = 0;
    let mut current = batch.initial_head_position;

    let (left, right): (Vec<u32>, Vec<u32>) =
        reqs.iter().partition(|&&r| r < batch.initial_head_position);

    match batch.direction {
        Direction::Right => {
            for &r in &right {
                total += (r as i32 - current as i32).unsigned_abs();
                current = r;
            }
            for &r in left.iter().rev() {
                total += (current as i32 - r as i32).unsigned_abs();
                current = r;
            }
        }
        Direction::Left => {
            for &r in left.iter().rev() {
                total += (current as i32 - r as i32).unsigned_abs();
                current = r;
            }
            for &r in &right {
                total += (r as i32 - current as i32).unsigned_abs();
                current = r;
            }
        }
    }

    total
}

fn c_look(batch: &Batch) -> u32 {
    let mut reqs = batch.requests.to_vec();
    reqs.sort();
    let mut total = 0;
    let mut current = batch.initial_head_position;

    let right: Vec<u32> = reqs
        .iter()
        .cloned()
        .filter(|&r| r >= batch.initial_head_position)
        .collect();
    let left: Vec<u32> = reqs
        .iter()
        .cloned()
        .filter(|&r| r < batch.initial_head_position)
        .collect();

    for r in &right {
        total += (*r as i32 - current as i32).unsigned_abs();
        current = *r;
    }

    if !left.is_empty() {
        total += (current as i32 - left[0] as i32).unsigned_abs();
        current = left[0];
        for r in &left {
            if *r != left[0] {
                total += (*r as i32 - current as i32).unsigned_abs();
                current = *r;
            }
        }
    }

    total
}

fn brute_force(batch: &Batch) -> u32 {
    batch
        .requests
        .iter()
        .permutations(batch.requests.len())
        .map(|perm| {
            let mut total: u32 = 0;
            let mut current: u32 = batch.initial_head_position;
            for &&r in &perm {
                total += (current as i32 - r as i32).unsigned_abs();
                current = r;
            }
            total
        })
        .min()
        .unwrap()
}

fn main() {
    let mut rng = rand::rng();

    let num_batches: usize = 1000;
    let batch_size: u32 = 8;

    let mut batches: Vec<Batch> = Vec::new();

    for _ in 0..num_batches {
        let cyl: u32 = rng.random_range(100..500);
        batches.push(Batch {
            num_cylinders: cyl,
            initial_head_position: rng.random_range(0..cyl),
            direction: if rng.random_bool(0.5) {
                Direction::Right
            } else {
                Direction::Left
            },
            requests: (0..batch_size).map(|_| rng.random_range(0..cyl)).collect(),
        });
    }

    let mut fcfs_count: u32 = 0;
    for batch in &batches {
        fcfs_count += fcfs(batch);
    }
    println!("FCFS    : {:8} total head movements", fcfs_count);

    let mut sstf_count: u32 = 0;
    for batch in &batches {
        sstf_count += sstf(batch);
    }
    println!("SSTF    : {:8} total head movements", sstf_count);

    let mut scan_count: u32 = 0;
    for batch in &batches {
        scan_count += scan(batch);
    }
    println!("SCAN    : {:8} total head movements", scan_count);

    let mut cscan_count: u32 = 0;
    for batch in &batches {
        cscan_count += c_scan(batch);
    }
    println!("C-SCAN  : {:8} total head movements", cscan_count);

    let mut look_count: u32 = 0;
    for batch in &batches {
        look_count += look(batch);
    }
    println!("LOOK    : {:8} total head movements", look_count);

    let mut clook_count: u32 = 0;
    for batch in &batches {
        clook_count += c_look(batch);
    }
    println!("C-LOOK  : {:8} total head movements", clook_count);

    if batch_size > 10 {
        println!("Skipping optimal because it would require too much time to simulate.");
    } else {
        let mut brute_force_count: u32 = 0;
        for batch in &batches {
            brute_force_count += brute_force(batch);
        }
        println!("Optimal : {:8} total head movements", brute_force_count);
    }
}
