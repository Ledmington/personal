use std::fmt::Display;

#[derive(Copy, Clone)]
pub(crate) enum CellState {
    Unknown { priority: u8 },
    Water,
    Hit,       // hit on a ship that is not yet destroyed
    Destroyed, // hit on a destroyed ship
}

impl Display for CellState {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            // Every variant must be exactly 2 columns wide (width specifiers are ignored by write!)
            CellState::Unknown { priority } => write!(f, "{:>2}", priority),
            CellState::Water => write!(f, "\x1b[90m██\x1b[0m"),
            CellState::Hit | CellState::Destroyed => write!(f, " \x1b[31m✗\x1b[0m"),
        }
    }
}

#[derive(Clone, Copy)]
pub(crate) struct ShipInfo {
    pub(crate) length: usize,
    pub(crate) amount: usize,
}

pub(crate) struct Grid {
    pub(crate) size: usize,
    pub(crate) grid: Vec<Vec<CellState>>,
    pub(crate) suggestion: Option<(usize, usize)>,
    /// Cell currently selected in the TUI
    pub(crate) cursor: Option<(usize, usize)>,
    pub(crate) ship_info: Vec<ShipInfo>,
}

impl Grid {
    /// Records a shot. A destroyed ship is every orthogonally connected hit cell; all cells around it become water
    /// and one ship of that length is removed from `ship_info`.
    pub(crate) fn shoot(&mut self, row: usize, col: usize, state: CellState) {
        self.grid[row][col] = state;
        if !matches!(state, CellState::Destroyed) {
            return;
        }
        let mut stack = vec![(row, col)];
        let mut length = 0;
        while let Some((r, c)) = stack.pop() {
            length += 1;
            for dr in -1i32..=1 {
                for dc in -1i32..=1 {
                    let (nr, nc) = (r as i32 + dr, c as i32 + dc);
                    if (nr, nc) == (r as i32, c as i32)
                        || nr < 0
                        || nc < 0
                        || nr >= self.size as i32
                        || nc >= self.size as i32
                    {
                        continue;
                    }
                    let (nr, nc) = (nr as usize, nc as usize);
                    match self.grid[nr][nc] {
                        CellState::Unknown { .. } => self.grid[nr][nc] = CellState::Water,
                        // only orthogonal neighbours belong to the same ship
                        CellState::Hit if dr == 0 || dc == 0 => {
                            self.grid[nr][nc] = CellState::Destroyed;
                            stack.push((nr, nc));
                        }
                        _ => {}
                    }
                }
            }
        }

        // shortcut: a length that is not in ship_info (or already at 0) is ignored, upgrade if the user needs a warning
        if let Some(info) = self.ship_info.iter_mut().find(|s| s.length == length) {
            info.amount = info.amount.saturating_sub(1);
        }
    }

    pub(crate) fn new(grid_size: usize, ships: Vec<ShipInfo>) -> Grid {
        assert!(grid_size > 0);
        assert!(!ships.is_empty());
        for info in ships.iter() {
            assert!(info.amount > 0);
            assert!(info.length > 0);
        }
        Grid {
            size: grid_size,
            grid: vec![vec![CellState::Unknown { priority: 0 }; grid_size]; grid_size],
            suggestion: None,
            cursor: None,
            ship_info: ships,
        }
    }
}

pub(crate) fn print_grid(grid: &Grid) {
    const UPPER_LEFT_CORNER: char = '┌';
    const UPPER_RIGHT_CORNER: char = '┐';
    const LOWER_LEFT_CORNER: char = '└';
    const LOWER_RIGHT_CORNER: char = '┘';
    const TOP_T: char = '┬';
    const BOTTOM_T: char = '┴';
    const LEFT_T: char = '├';
    const RIGHT_T: char = '┤';
    const CROSS: char = '┼';
    const VERTICAL_LINE: char = '│';
    const HORIZONTAL_LINE: char = '─';

    print!("    ");
    for i in 0..grid.size {
        print!("{:>2} ", i);
    }
    println!();

    print!("   {}", UPPER_LEFT_CORNER);
    for col in 0..grid.size {
        let after = if col == (grid.size - 1) {
            UPPER_RIGHT_CORNER
        } else {
            TOP_T
        };
        print!("{}{}{}", HORIZONTAL_LINE, HORIZONTAL_LINE, after);
    }
    println!();
    for i in 0..grid.size {
        print!(" {} {}", (b'A' + i as u8) as char, VERTICAL_LINE);
        for j in 0..grid.size {
            let style = match (grid.cursor == Some((i, j)), grid.suggestion == Some((i, j))) {
                (true, true) => "\x1b[7;33m",
                (true, false) => "\x1b[7m",
                (false, true) => "\x1b[33m",
                (false, false) => "",
            };
            print!("{style}{}\x1b[0m{}", grid.grid[i][j], VERTICAL_LINE);
        }
        println!();

        if i < grid.size - 1 {
            print!("   {}", LEFT_T);
            for col in 0..grid.size {
                let after = if col == (grid.size - 1) {
                    RIGHT_T
                } else {
                    CROSS
                };
                print!("{}{}{}", HORIZONTAL_LINE, HORIZONTAL_LINE, after);
            }
            println!();
        }
    }

    print!("   {}", LOWER_LEFT_CORNER);
    for col in 0..grid.size {
        let after = if col == (grid.size - 1) {
            LOWER_RIGHT_CORNER
        } else {
            BOTTOM_T
        };
        print!("{}{}{}", HORIZONTAL_LINE, HORIZONTAL_LINE, after);
    }
    println!();
}

#[cfg(test)]
#[allow(clippy::needless_range_loop)] // (r, c) are used as coordinates, not just indices
mod tests {
    use std::cmp::max;

    use crate::grid::ShipInfo;

    use super::*;

    #[test]
    fn all_unknown_by_default() {
        let grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 1,
                amount: 1,
            }],
        );
        assert!(
            grid.grid
                .iter()
                .all(|row| row.iter().all(|x| matches!(x, CellState::Unknown { .. })))
        );
    }

    #[test]
    fn shot_in_the_water() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 1,
                amount: 1,
            }],
        );

        let (shot_row, shot_col) = (3, 3);
        grid.shoot(shot_row, shot_col, CellState::Water);

        for r in 0..grid.size {
            for c in 0..grid.size {
                if (r, c) == (shot_row, shot_col) {
                    assert!(matches!(grid.grid[r][c], CellState::Water), "({r},{c})");
                } else {
                    assert!(
                        matches!(grid.grid[r][c], CellState::Unknown { .. }),
                        "({r},{c})"
                    );
                }
            }
        }
    }

    #[test]
    fn shot_on_a_ship() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 1,
                amount: 1,
            }],
        );

        let (shot_row, shot_col) = (3, 3);
        grid.shoot(shot_row, shot_col, CellState::Hit);

        for r in 0..grid.size {
            for c in 0..grid.size {
                if (r, c) == (shot_row, shot_col) {
                    assert!(matches!(grid.grid[r][c], CellState::Hit), "({r},{c})");
                } else {
                    assert!(
                        matches!(grid.grid[r][c], CellState::Unknown { .. }),
                        "({r},{c})"
                    );
                }
            }
        }
    }

    #[test]
    fn destroyed_single_ship() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 1,
                amount: 1,
            }],
        );

        let (shot_row, shot_col) = (3, 3);
        grid.shoot(shot_row, shot_col, CellState::Destroyed);

        for r in 0..grid.size {
            for c in 0..grid.size {
                if (r, c) == (shot_row, shot_col) {
                    assert!(matches!(grid.grid[r][c], CellState::Destroyed), "({r},{c})");
                } else if max(
                    (r as i32 - shot_row as i32).abs(),
                    (c as i32 - shot_col as i32).abs(),
                ) == 1
                {
                    assert!(matches!(grid.grid[r][c], CellState::Water), "({r},{c})");
                } else {
                    assert!(
                        matches!(grid.grid[r][c], CellState::Unknown { .. }),
                        "({r},{c})"
                    );
                }
            }
        }
    }

    #[test]
    fn destroying_a_ship_updates_ship_info() {
        let mut grid = Grid::new(
            10,
            vec![
                ShipInfo {
                    length: 1,
                    amount: 1,
                },
                ShipInfo {
                    length: 2,
                    amount: 2,
                },
            ],
        );
        grid.shoot(3, 3, CellState::Hit);
        grid.shoot(3, 4, CellState::Destroyed);

        let amount = |len: usize| {
            grid.ship_info
                .iter()
                .find(|s| s.length == len)
                .unwrap()
                .amount
        };
        assert_eq!(amount(2), 1);
        assert_eq!(amount(1), 1);
    }

    #[test]
    fn destroyed_long_ship() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 2,
                amount: 1,
            }],
        );

        let (shot1_row, shot1_col) = (3, 3);
        grid.shoot(shot1_row, shot1_col, CellState::Hit);
        let (shot2_row, shot2_col) = (3, 4);
        grid.shoot(shot2_row, shot2_col, CellState::Destroyed);

        let ship = [(shot1_row, shot1_col), (shot2_row, shot2_col)];
        for r in 0..grid.size {
            for c in 0..grid.size {
                // distance to the closest ship cell (0 = part of the ship)
                let dist = ship
                    .iter()
                    .map(|&(sr, sc)| {
                        max((r as i32 - sr as i32).abs(), (c as i32 - sc as i32).abs())
                    })
                    .min()
                    .unwrap();
                if dist == 0 {
                    assert!(matches!(grid.grid[r][c], CellState::Destroyed), "({r},{c})");
                } else if dist == 1 {
                    assert!(matches!(grid.grid[r][c], CellState::Water), "({r},{c})");
                } else {
                    assert!(
                        matches!(grid.grid[r][c], CellState::Unknown { .. }),
                        "({r},{c})"
                    );
                }
            }
        }
    }
}
