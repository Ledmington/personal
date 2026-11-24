use std::io::{self, Write, stdout};

use crossterm::{
    cursor::{Hide, MoveTo, Show},
    event::{self, Event, KeyCode, KeyEvent, KeyEventKind, KeyModifiers},
    execute,
    terminal::{
        Clear, ClearType, EnterAlternateScreen, LeaveAlternateScreen, disable_raw_mode,
        enable_raw_mode,
    },
};

use crate::{
    grid::{CellState, Grid, print_grid},
    priority::{compute_priorities, find_best_spot},
};

const OPTIONS: [(&str, CellState); 3] = [
    ("Water", CellState::Water),
    ("Hit", CellState::Hit),
    ("Destroyed", CellState::Destroyed),
];

/// Restores the terminal on exit, even if the app panics.
struct TerminalGuard;

impl TerminalGuard {
    fn new() -> io::Result<Self> {
        enable_raw_mode()?;
        execute!(stdout(), EnterAlternateScreen, Hide)?;
        Ok(Self)
    }
}

impl Drop for TerminalGuard {
    fn drop(&mut self) {
        let _ = execute!(stdout(), Show, LeaveAlternateScreen);
        let _ = disable_raw_mode();
    }
}

fn draw(grid: &Grid, menu: Option<usize>, message: &str) -> io::Result<()> {
    // shortcut: print_grid uses println!, whose "\n" needs cooked mode to return the carriage;
    // raw mode is switched off while drawing. Upgrade: make print_grid return a String.
    disable_raw_mode()?;
    execute!(stdout(), MoveTo(0, 0), Clear(ClearType::All))?;
    print_grid(grid);
    println!();
    match (menu, grid.cursor) {
        (Some(sel), Some((r, c))) => {
            print!("Result of {}{}:", (b'A' + r as u8) as char, c);
            for (i, (name, _)) in OPTIONS.iter().enumerate() {
                if i == sel {
                    print!(" \x1b[7m {name} \x1b[0m");
                } else {
                    print!("  {name} ");
                }
            }
            println!("\n←/→ choose, Enter confirm, Esc cancel");
        }
        _ => println!("Arrows: move, Enter: shoot at this cell, q: quit"),
    }
    println!("{message}");
    stdout().flush()?;
    enable_raw_mode()
}

pub(crate) fn run(mut grid: Grid) -> io::Result<()> {
    let guard = TerminalGuard::new()?;
    compute_priorities(&mut grid);
    grid.suggestion = find_best_spot(&grid);
    let mut cursor = grid.suggestion.unwrap_or_default();
    let mut menu: Option<usize> = None;
    let mut message = String::new();

    loop {
        grid.cursor = Some(cursor);
        draw(&grid, menu, &message)?;
        message.clear();

        let Event::Key(KeyEvent {
            code,
            modifiers,
            kind: KeyEventKind::Press,
            ..
        }) = event::read()?
        else {
            continue;
        };
        if code == KeyCode::Char('c') && modifiers.contains(KeyModifiers::CONTROL) {
            return Ok(());
        }

        let n = OPTIONS.len();
        match (menu, code) {
            (_, KeyCode::Char('q')) | (None, KeyCode::Esc) => return Ok(()),
            (Some(_), KeyCode::Esc) => menu = None,
            (Some(sel), KeyCode::Left | KeyCode::Up) => menu = Some((sel + n - 1) % n),
            (Some(sel), KeyCode::Right | KeyCode::Down) => menu = Some((sel + 1) % n),
            (Some(sel), KeyCode::Enter) => {
                grid.shoot(cursor.0, cursor.1, OPTIONS[sel].1);
                compute_priorities(&mut grid);
                grid.suggestion = find_best_spot(&grid);
                cursor = grid.suggestion.unwrap_or(cursor);
                menu = None;
                if grid.ship_info.iter().all(|s| s.amount == 0) {
                    drop(guard); // leave the alternate screen so the message stays visible
                    println!("You should have won by now.");
                    return Ok(());
                }
            }
            (None, KeyCode::Enter) => {
                if matches!(grid.grid[cursor.0][cursor.1], CellState::Unknown { .. }) {
                    menu = Some(0);
                } else {
                    message = "That cell was already shot.".into();
                }
            }
            (None, KeyCode::Left) => cursor.1 = cursor.1.saturating_sub(1),
            (None, KeyCode::Right) => cursor.1 = (cursor.1 + 1).min(grid.size - 1),
            (None, KeyCode::Up) => cursor.0 = cursor.0.saturating_sub(1),
            (None, KeyCode::Down) => cursor.0 = (cursor.0 + 1).min(grid.size - 1),
            _ => {}
        }
    }
}
