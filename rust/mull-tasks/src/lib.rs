use indicatif::{ProgressBar, ProgressStyle, TermLike};
use mull_core::{diag_info, diagnostics::MullDiagnostics, utils::format_elapsed};
use rayon::prelude::*;
use std::{
    cmp::min,
    io::{self, Write},
    time::{Duration, Instant},
};

fn is_terminal() -> bool {
    console::Term::stdout().is_term()
}

#[derive(Debug)]
struct LinePrinter;

// Used in non-term environments as a hack to make indicatif do the sane thing
impl TermLike for LinePrinter {
    fn width(&self) -> u16 {
        80
    }
    fn move_cursor_up(&self, _n: usize) -> io::Result<()> {
        Ok(())
    }
    fn move_cursor_down(&self, _n: usize) -> io::Result<()> {
        Ok(())
    }
    fn move_cursor_right(&self, _n: usize) -> io::Result<()> {
        Ok(())
    }
    fn move_cursor_left(&self, _n: usize) -> io::Result<()> {
        Ok(())
    }
    fn write_line(&self, s: &str) -> io::Result<()> {
        println!("{s}");
        Ok(())
    }
    fn write_str(&self, s: &str) -> io::Result<()> {
        if s == "\r" {
            return Ok(());
        }
        print!("{}", s);
        Ok(())
    }
    fn clear_line(&self) -> io::Result<()> {
        Ok(()) // No-op, flush() adds the newline
    }
    fn flush(&self) -> io::Result<()> {
        println!(); // End each update with a newline
        io::stdout().flush()
    }
}

fn create_progress_bar(total: usize) -> ProgressBar {
    let draw_target = if is_terminal() {
        indicatif::ProgressDrawTarget::term_like(Box::new(console::Term::stdout()))
    } else {
        indicatif::ProgressDrawTarget::term_like(Box::new(LinePrinter))
    };
    let pb = ProgressBar::with_draw_target(Some(total as u64), draw_target);
    pb.set_style(
        ProgressStyle::default_bar()
            .template("       [{bar:32}] {pos}/{len}. {msg}")
            .unwrap()
            .progress_chars("#-"),
    );
    if is_terminal() {
        pb.enable_steady_tick(Duration::from_millis(50));
    }
    pb
}

pub fn run_task<T, F>(diag: &MullDiagnostics, name: &str, task: F) -> T
where
    F: FnOnce() -> T,
{
    diag_info!(diag, "{} (threads: 1)", name);

    let pb = create_progress_bar(1);
    let start = Instant::now();
    diag.mark_progress_active();

    let result = task();

    pb.inc(1);
    pb.finish_with_message(format!("Finished in {}", format_elapsed(start.elapsed())));
    diag.mark_progress_finished();

    result
}

pub fn run_parallel_task<T, I, F>(
    diag: &MullDiagnostics,
    name: &str,
    workers: usize,
    items: I,
    task: F,
) -> Vec<T>
where
    T: Send,
    I: IntoParallelIterator,
    I::Item: Send,
    F: Fn(usize, I::Item) -> T + Sync,
{
    let items: Vec<_> = items.into_par_iter().collect();
    let workers = min(workers, items.len());
    let total = items.len();
    let start = Instant::now();

    diag_info!(diag, "{} (threads: {})", name, workers);

    let pb = create_progress_bar(total);
    diag.mark_progress_active();

    let pool = rayon::ThreadPoolBuilder::new()
        .num_threads(workers)
        .build()
        .unwrap();

    let results: Vec<T> = pool.install(|| {
        items
            .into_par_iter()
            .enumerate()
            .map(|(idx, item)| {
                let result = task(idx, item);
                pb.inc(1);
                pb.set_message(format_elapsed(pb.elapsed()));
                result
            })
            .collect()
    });

    pb.finish_with_message(format!("Finished in {}", format_elapsed(start.elapsed())));
    diag.mark_progress_finished();

    results
}
