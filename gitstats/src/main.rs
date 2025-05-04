use chrono::{DateTime, Utc};
use plotters::prelude::*;
use std::{
    collections::BTreeMap,
    env,
    path::PathBuf,
    process::Command,
    time::{Duration, Instant},
};

#[derive(Debug)]
struct CommitStats {
    files: usize,
    lines: usize,
}

#[derive(Debug)]
struct Commit {
    hash: String,
    timestamp: i64,
}

fn get_commit_list(default_branch: &str) -> Vec<Commit> {
    // println!("  git log --pretty=format:%H|%at {}", default_branch);
    let output = Command::new("git")
        .args(["log", "--pretty=format:%H|%at", default_branch])
        .output()
        .expect("Failed to run git log.");

    let log = String::from_utf8_lossy(&output.stdout);
    log.lines()
        .filter_map(|line| {
            let parts: Vec<&str> = line.split('|').collect();
            if parts.len() == 2 {
                Some(Commit {
                    hash: parts[0].to_string(),
                    timestamp: parts[1].parse::<i64>().ok()?,
                })
            } else {
                eprintln!("Line {} was formatted incorrectly.", line);
                None
            }
        })
        .collect()
}

fn count_lines_and_files_at_commit(commit: &str) -> (usize, usize) {
    // println!("  git ls-tree -r --name-only {}", commit);
    let output = Command::new("git")
        .args(["ls-tree", "-r", "--name-only", commit])
        .output()
        .expect("Failed to run git ls-tree");

    if !output.status.success() {
        eprintln!("git ls-tree failed for commit {}.", commit);
        return (0, 0);
    }

    let files = String::from_utf8_lossy(&output.stdout);
    let mut total_files = 0;
    let mut total_lines = 0;

    for path in files.lines() {
        // println!("  git show {}:{}", commit, path);
        let file_output = Command::new("git")
            .args(["show", &format!("{}:{}", commit, path)])
            .output();

        if let Ok(output) = file_output {
            if output.status.success() {
                total_files += 1;
                let contents = String::from_utf8_lossy(&output.stdout);
                let n = contents.lines().count();
                total_lines += n;
            }
        }
    }

    (total_files, total_lines)
}

fn get_default_branch() -> Option<String> {
    // println!("  git symbolic-ref refs/remotes/origin/HEAD");
    let output = Command::new("git")
        .args(["symbolic-ref", "refs/remotes/origin/HEAD"])
        .output()
        .ok()?;

    if !output.status.success() {
        return None;
    }

    let full_ref = String::from_utf8_lossy(&output.stdout);
    // Example: refs/remotes/origin/main -> main
    full_ref
        .trim()
        .split('/')
        .next_back()
        .map(|s| s.to_string())
}

fn restore_main_branch(default_branch: &str) {
    // println!("  git checkout {}", default_branch);
    Command::new("git")
        .args(["checkout", default_branch])
        .output()
        .expect("Failed to checkout default branch.");
}

/// Returns the absolute path to the root of the git repo
fn get_git_root() -> Option<PathBuf> {
    // println!("  git rev-parse show-toplevel");
    let output = Command::new("git")
        .args(["rev-parse", "--show-toplevel"])
        .output()
        .ok()?;

    if !output.status.success() {
        return None;
    }

    let path = String::from_utf8_lossy(&output.stdout);
    Some(PathBuf::from(path.trim()))
}

fn plot_files(
    stats_map: &BTreeMap<DateTime<Utc>, CommitStats>,
) -> Result<(), Box<dyn std::error::Error>> {
    let root = BitMapBackend::new("files.png", (1024, 768)).into_drawing_area();
    root.fill(&WHITE)?;

    let mut chart = ChartBuilder::on(&root)
        .caption("Number of Files Over Time", ("sans-serif", 30))
        .margin(10)
        .x_label_area_size(40)
        .y_label_area_size(60)
        .build_cartesian_2d(
            stats_map.keys().cloned().collect::<Vec<_>>()[0]
                ..*stats_map
                    .keys()
                    .cloned()
                    .collect::<Vec<_>>()
                    .last()
                    .unwrap(),
            0u32..stats_map.values().map(|s| s.files).max().unwrap_or(100) as u32,
        )?;

    chart.configure_mesh().x_labels(10).y_labels(10).draw()?;

    chart
        .draw_series(LineSeries::new(
            stats_map
                .iter()
                .map(|(date, stat)| (*date, stat.files as u32)),
            &BLUE,
        ))?
        .label("Total Files")
        .legend(|(x, y)| PathElement::new(vec![(x, y), (x + 20, y)], BLUE));

    chart.configure_series_labels().border_style(BLACK).draw()?;

    Ok(())
}

fn plot_lines(
    stats_map: &BTreeMap<DateTime<Utc>, CommitStats>,
) -> Result<(), Box<dyn std::error::Error>> {
    let root = BitMapBackend::new("lines.png", (1024, 768)).into_drawing_area();
    root.fill(&WHITE)?;

    let mut chart = ChartBuilder::on(&root)
        .caption("Lines of Code Over Time", ("sans-serif", 30))
        .margin(10)
        .x_label_area_size(40)
        .y_label_area_size(60)
        .build_cartesian_2d(
            stats_map.keys().cloned().collect::<Vec<_>>()[0]
                ..*stats_map
                    .keys()
                    .cloned()
                    .collect::<Vec<_>>()
                    .last()
                    .unwrap(),
            0u32..stats_map.values().map(|s| s.lines).max().unwrap_or(100) as u32,
        )?;

    chart.configure_mesh().x_labels(10).y_labels(10).draw()?;

    chart
        .draw_series(LineSeries::new(
            stats_map
                .iter()
                .map(|(date, stat)| (*date, stat.lines as u32)),
            &RED,
        ))?
        .label("Total Lines")
        .legend(|(x, y)| PathElement::new(vec![(x, y), (x + 20, y)], RED));

    chart.configure_series_labels().border_style(BLACK).draw()?;

    Ok(())
}

fn format_duration(dur: Duration) -> String {
    let mut t = dur.as_secs();
    let secs = t % 60;
    t /= 60;
    let mins = t % 60;
    t /= 60;
    let hours = t % 60;
    format!("{}h {}m {}s", hours, mins, secs)
}

fn run_git_stats() -> BTreeMap<DateTime<Utc>, CommitStats> {
    println!("Generating Git statistics...");

    let default_branch: String = get_default_branch().expect("Failed to get default branch.");

    let all_commits = get_commit_list(&default_branch);
    let mut stats_map: BTreeMap<DateTime<Utc>, CommitStats> = BTreeMap::new();

    let num_commits = all_commits.len();
    let start = Instant::now();
    for (i, commit) in all_commits.iter().rev().enumerate() {
        let hash: &str = &commit.hash;
        let timestamp: i64 = commit.timestamp;

        let (files, lines) = count_lines_and_files_at_commit(hash);

        let date = DateTime::<Utc>::from_timestamp(timestamp, 0).unwrap();

        stats_map.insert(date, CommitStats { files, lines });

        let elapsed = start.elapsed();
        let avg_time = elapsed / (i + 1) as u32;
        let remaining = num_commits - i - 1;
        let eta = avg_time * remaining as u32;

        println!(
            "[{}/{}] Commit {}: {} files, {} lines. ETA: {}",
            i + 1,
            num_commits,
            &hash[..7],
            files,
            lines,
            format_duration(eta)
        );
    }

    restore_main_branch(&default_branch);

    stats_map
}

fn is_inside_git_repo() -> bool {
    // println!("  git rev-parse --is-inside-work-tree");
    let output = Command::new("git")
        .args(["rev-parse", "--is-inside-work-tree"])
        .output();

    match output {
        Ok(output) if output.status.success() => {
            String::from_utf8_lossy(&output.stdout).trim() == "true"
        }
        _ => false,
    }
}

fn main() {
    if !is_inside_git_repo() {
        eprintln!("Error: This tool must be run inside a Git repository.");
        std::process::exit(1);
    }

    let original_dir = env::current_dir().expect("Cannot determine current directory");

    let git_root = get_git_root().expect("Not inside a Git repository");
    env::set_current_dir(&git_root).expect("Failed to change to Git root");

    let stats_map: BTreeMap<DateTime<Utc>, CommitStats> = run_git_stats();

    env::set_current_dir(original_dir).expect("Failed to restore original directory");

    println!("\nPlotting data...");

    plot_files(&stats_map).expect("Plotting failed.");
    println!("Plot saved as files.png");

    plot_lines(&stats_map).expect("Plotting failed.");
    println!("Plot saved as lines.png");
}
