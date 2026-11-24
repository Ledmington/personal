use std::collections::HashSet;

use crate::grid::{CellState, Grid};

/// Priority added to the cells that extend a run of two or more hits
const RUN_END_BONUS: u8 = 20;
/// Priority added to the neighbours of a lone hit
const LONE_HIT_BONUS: u8 = 10;

fn increment_priority(cell: &mut CellState) {
    match cell {
        CellState::Unknown { priority } => *priority += 1,
        _ => unreachable!(),
    }
}

fn compute_priority_of_ships(grid: &mut Grid, ship_length: usize) {
    assert!(ship_length >= 2);

    for i in 0..grid.size {
        for j in 0..grid.size {
            // vertical
            if i < (grid.size - (ship_length - 1)) {
                let mut valid = true;
                for k in 0..ship_length {
                    if !matches!(grid.grid[i + k][j], CellState::Unknown { .. }) {
                        valid = false;
                        break;
                    }
                }
                if valid {
                    for k in 0..ship_length {
                        increment_priority(&mut grid.grid[i + k][j]);
                    }
                }
            }

            // horizontal
            if j < (grid.size - (ship_length - 1)) {
                let mut valid = true;
                for k in 0..ship_length {
                    if !matches!(grid.grid[i][j + k], CellState::Unknown { .. }) {
                        valid = false;
                        break;
                    }
                }
                if valid {
                    for k in 0..ship_length {
                        increment_priority(&mut grid.grid[i][j + k]);
                    }
                }
            }
        }
    }
}

/// A single-cell ship fits anywhere: a cell is worth the number of unknown neighbours (diagonals included) it has.
fn compute_priority_of_single_cells(grid: &mut Grid) {
    for i in 0..grid.size {
        for j in 0..grid.size {
            if !matches!(grid.grid[i][j], CellState::Unknown { .. }) {
                continue;
            }
            let unknown_neighbours = (-1isize..=1)
                .flat_map(|di| (-1isize..=1).map(move |dj| (di, dj)))
                .filter(|&d| d != (0, 0))
                .filter(|&(di, dj)| {
                    let (r, c) = (i.wrapping_add_signed(di), j.wrapping_add_signed(dj));
                    matches!(
                        grid.grid.get(r).and_then(|row| row.get(c)),
                        Some(CellState::Unknown { .. })
                    )
                })
                .count() as u8;
            if let CellState::Unknown { priority } = &mut grid.grid[i][j] {
                *priority += unknown_neighbours;
            }
        }
    }
}

fn compute_priority_of_hits(grid: &mut Grid) {
    // Collect all hits
    let mut hits_positions: HashSet<(usize, usize)> = HashSet::new();
    for (i, row) in grid.grid.iter().enumerate() {
        for (j, cell) in row.iter().enumerate() {
            if matches!(cell, CellState::Hit) {
                hits_positions.insert((i, j));
            }
        }
    }

    if hits_positions.is_empty() {
        return;
    }

    // Two or more hits in a row (or column): boost the cells that extend the run at both ends.
    // Every processed hit is removed from the set; the ones in no run are lone hits.
    let mut singles = Vec::new();
    while let Some(&(i, j)) = hits_positions.iter().next() {
        for (di, dj) in [(0usize, 1usize), (1, 0)] {
            // find the whole run through (i, j)
            let (mut si, mut sj) = (i, j);
            while let Some(p) = si
                .checked_sub(di)
                .zip(sj.checked_sub(dj))
                .filter(|p| hits_positions.contains(p))
            {
                (si, sj) = p;
            }
            let (mut ei, mut ej) = (i, j);
            while hits_positions.contains(&(ei + di, ej + dj)) {
                ei += di;
                ej += dj;
            }
            if (si, sj) == (ei, ej) {
                continue;
            }

            let before = si.checked_sub(di).zip(sj.checked_sub(dj));
            for (r, c) in [before, Some((ei + di, ej + dj))].into_iter().flatten() {
                if let Some(CellState::Unknown { priority }) =
                    grid.grid.get_mut(r).and_then(|row| row.get_mut(c))
                {
                    *priority += RUN_END_BONUS;
                }
            }

            // shortcut: a hit shared by a horizontal and a vertical run only counts for the first, ships can't touch anyway
            let (mut ri, mut rj) = (si, sj);
            while (ri, rj) != (ei + di, ej + dj) {
                hits_positions.remove(&(ri, rj));
                ri += di;
                rj += dj;
            }
        }
        if hits_positions.remove(&(i, j)) {
            singles.push((i, j));
        }
    }

    // A lone hit: the ship continues in one of the 4 orthogonal neighbours
    for (i, j) in singles {
        for (r, c) in [
            (i.wrapping_sub(1), j),
            (i + 1, j),
            (i, j.wrapping_sub(1)),
            (i, j + 1),
        ] {
            if let Some(CellState::Unknown { priority }) =
                grid.grid.get_mut(r).and_then(|row| row.get_mut(c))
            {
                *priority += LONE_HIT_BONUS;
            }
        }
    }
}

pub(crate) fn compute_priorities(grid: &mut Grid) {
    // Set all priorities to 0
    for i in 0..grid.size {
        for j in 0..grid.size {
            match &mut grid.grid[i][j] {
                CellState::Unknown { priority } => *priority = 0,
                _ => continue,
            }
        }
    }

    // Ships of a length that was fully destroyed no longer count
    let lengths: Vec<usize> = grid
        .ship_info
        .iter()
        .filter(|s| s.amount > 0 && s.length >= 2)
        .map(|s| s.length)
        .collect();
    for length in lengths {
        compute_priority_of_ships(grid, length);
    }

    // Only when every remaining ship is a single cell
    let mut remaining = grid.ship_info.iter().filter(|s| s.amount > 0).peekable();
    if remaining.peek().is_some() && remaining.all(|s| s.length == 1) {
        compute_priority_of_single_cells(grid);
    }

    compute_priority_of_hits(grid);
}

/// The unknown cell with the highest priority (the first one on ties, even if the priority is 0).
/// None when no unknown cell is left.
pub(crate) fn find_best_spot(grid: &Grid) -> Option<(usize, usize)> {
    let mut best: Option<(u8, (usize, usize))> = None;
    for (i, row) in grid.grid.iter().enumerate() {
        for (j, cell) in row.iter().enumerate() {
            if let CellState::Unknown { priority: p } = *cell
                && best.is_none_or(|(best_p, _)| p > best_p)
            {
                best = Some((p, (i, j)));
            }
        }
    }
    best.map(|(_, pos)| pos)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::grid::ShipInfo;

    #[test]
    fn run_of_hits_boosts_both_ends() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 3,
                amount: 1,
            }],
        );
        grid.shoot(3, 3, CellState::Hit);
        grid.shoot(3, 4, CellState::Hit);
        compute_priorities(&mut grid);

        let p = |r: usize, c: usize| match grid.grid[r][c] {
            CellState::Unknown { priority } => priority,
            _ => panic!("({r},{c}) not unknown"),
        };
        assert!(p(3, 2) >= RUN_END_BONUS && p(3, 5) >= RUN_END_BONUS);
        assert!(p(2, 3) < RUN_END_BONUS && p(4, 4) < RUN_END_BONUS);
    }

    #[test]
    fn lone_hit_boosts_its_neighbours() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 3,
                amount: 1,
            }],
        );
        grid.shoot(0, 0, CellState::Hit);
        compute_priorities(&mut grid);

        let p = |r: usize, c: usize| match grid.grid[r][c] {
            CellState::Unknown { priority } => priority,
            _ => panic!("({r},{c}) not unknown"),
        };
        assert!(p(0, 1) >= LONE_HIT_BONUS && p(1, 0) >= LONE_HIT_BONUS);
        assert!(p(1, 1) < LONE_HIT_BONUS);
    }

    #[test]
    fn lone_hit_with_water_keeps_the_free_neighbours_high() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 3,
                amount: 1,
            }],
        );
        grid.shoot(5, 5, CellState::Hit);
        grid.shoot(4, 5, CellState::Water);
        grid.shoot(6, 5, CellState::Water);
        compute_priorities(&mut grid);

        let p = |r: usize, c: usize| match grid.grid[r][c] {
            CellState::Unknown { priority } => priority,
            _ => panic!("({r},{c}) not unknown"),
        };
        assert!(p(5, 4) >= LONE_HIT_BONUS && p(5, 6) >= LONE_HIT_BONUS);
    }

    #[test]
    fn destroyed_lengths_are_not_considered_anymore() {
        let two = ShipInfo {
            length: 2,
            amount: 1,
        };
        let four = || ShipInfo {
            length: 4,
            amount: 1,
        };
        let mut both = Grid::new(10, vec![two, four()]);
        let mut only_four = Grid::new(10, vec![four()]);
        for g in [&mut both, &mut only_four] {
            g.shoot(0, 0, CellState::Hit);
            g.shoot(0, 1, CellState::Destroyed);
            compute_priorities(g);
        }
        // far from the destroyed ship, both grids must agree
        assert!(matches!(
            (both.grid[5][5], only_four.grid[5][5]),
            (CellState::Unknown { priority: a }, CellState::Unknown { priority: b }) if a == b
        ));
    }

    #[test]
    fn single_cell_ships_count_unknown_neighbours() {
        let single = || {
            vec![ShipInfo {
                length: 1,
                amount: 1,
            }]
        };
        let mut grid = Grid::new(10, single());
        grid.shoot(5, 6, CellState::Water);
        compute_priorities(&mut grid);

        let p = |g: &Grid, r: usize, c: usize| match g.grid[r][c] {
            CellState::Unknown { priority } => priority,
            _ => panic!("({r},{c}) not unknown"),
        };
        assert_eq!(p(&grid, 0, 0), 3); // corner
        assert_eq!(p(&grid, 0, 1), 5); // edge
        assert_eq!(p(&grid, 2, 2), 8); // interior
        assert_eq!(p(&grid, 5, 5), 7); // interior, one water neighbour

        // once the single-cell ships are all destroyed they stop counting
        let mut grid = Grid::new(10, single());
        grid.shoot(5, 5, CellState::Destroyed);
        compute_priorities(&mut grid);
        assert_eq!(p(&grid, 2, 2), 0);
    }

    #[test]
    fn single_cell_ships_are_ignored_while_longer_ones_remain() {
        let single = ShipInfo {
            length: 1,
            amount: 1,
        };
        let double = || ShipInfo {
            length: 2,
            amount: 1,
        };
        let mut mixed = Grid::new(10, vec![single, double()]);
        let mut only_double = Grid::new(10, vec![double()]);
        compute_priorities(&mut mixed);
        compute_priorities(&mut only_double);
        assert!(matches!(
            (mixed.grid[2][2], only_double.grid[2][2]),
            (CellState::Unknown { priority: a }, CellState::Unknown { priority: b }) if a == b
        ));
    }

    #[test]
    fn best_spot_can_have_priority_zero() {
        let mut grid = Grid::new(
            2,
            vec![ShipInfo {
                length: 1,
                amount: 1,
            }],
        );
        // priorities were never computed: every unknown cell is 0
        assert_eq!(find_best_spot(&grid), Some((0, 0)));
        // the suggestion must be an unknown cell, not the default (0, 0)
        grid.shoot(0, 0, CellState::Water);
        assert_eq!(find_best_spot(&grid), Some((0, 1)));
        for (r, c) in [(0, 1), (1, 0), (1, 1)] {
            grid.shoot(r, c, CellState::Water);
        }
        assert_eq!(find_best_spot(&grid), None);
    }

    #[test]
    fn hits_on_the_border_do_not_panic() {
        let mut grid = Grid::new(
            10,
            vec![ShipInfo {
                length: 3,
                amount: 1,
            }],
        );
        grid.shoot(0, 0, CellState::Hit);
        grid.shoot(0, 1, CellState::Hit);
        grid.shoot(9, 9, CellState::Hit);
        grid.shoot(8, 9, CellState::Hit);
        compute_priorities(&mut grid);
    }
}
