use indicatif::{ProgressBar, ProgressStyle};
use mull_core::{diag_info, diagnostics::MullDiagnostics, utils::format_elapsed};
use rayon::prelude::*;
use std::{
    cmp::min,
    time::{Duration, Instant},
};

fn create_progress_bar(total: usize) -> ProgressBar {
    let term = indicatif::ProgressDrawTarget::term_like(Box::new(console::Term::stdout()));
    let pb = ProgressBar::with_draw_target(Some(total as u64), term);
    pb.set_style(
        ProgressStyle::default_bar()
            .template("       [{bar:32}] {pos}/{len}. {msg}")
            .unwrap()
            .progress_chars("#-"),
    );
    pb.enable_steady_tick(Duration::from_millis(50));
    pb
}

pub fn run_task<T, F>(diag: &MullDiagnostics, name: &str, task: F) -> T
where
    F: FnOnce() -> T,
{
    diag_info!(diag, "{} (threads: 1)", name);

    let pb = create_progress_bar(1);
    diag.mark_progress_active();
    let start = Instant::now();

    let result = task();

    pb.inc(1);
    pb.finish_with_message(format!("Finished in {}", format_elapsed(start.elapsed())));
    diag.mark_progress_finished();
    if !console::Term::stdout().is_term() {
        println!();
    }

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

    pb.finish_with_message(format!("Finished in {}", format_elapsed(pb.elapsed())));
    diag.mark_progress_finished();
    if !console::Term::stdout().is_term() {
        println!();
    }

    results
}
