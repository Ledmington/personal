use clap::Parser;

use crate::grid::{Grid, ShipInfo};

mod grid;
mod priority;
mod tui;

fn interactive_mode() {
    let grid = Grid::new(
        10,
        vec![
            ShipInfo {
                length: 1,
                amount: 4,
            },
            ShipInfo {
                length: 2,
                amount: 3,
            },
            ShipInfo {
                length: 3,
                amount: 2,
            },
            ShipInfo {
                length: 4,
                amount: 1,
            },
        ],
    );
    tui::run(grid).expect("terminal error");
}

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Interactive mode, to use this tool during real-time battles
    #[arg(short, long, default_value_t = true)]
    interactive_mode: bool,
}

fn main() {
    let args = Args::parse();

    if args.interactive_mode {
        interactive_mode();
    } else {
        todo!();
    }
}
