use mull_core::{diag_info, diagnostics::MullDiagnostics, utils::format_elapsed};
use rayon::prelude::*;
use std::{
    cmp::min,
    sync::atomic::{AtomicUsize, Ordering},
    time::Instant,
};

fn is_terminal() -> bool {
    console::Term::stdout().is_term()
}

fn format_progress(current: usize, total: usize, msg: &str, is_term: bool) -> String {
    let bar_width = 32;
    let filled = (current as f64 / total as f64 * bar_width as f64) as usize;
    let empty = bar_width - filled;
    let bar = format!("{}{}", "#".repeat(filled), "-".repeat(empty));
    let prefix = if is_term { "\r" } else { "\n" };
    format!("{}       [{}] {}/{}. {}", prefix, bar, current, total, msg)
}

pub fn run_task<T, F>(diag: &MullDiagnostics, name: &str, task: F) -> T
where
    F: FnOnce() -> T,
{
    diag_info!(diag, "{} (threads: 1)", name);
    let start = Instant::now();

    let result = task();

    let msg = format!("Finished in {}", format_elapsed(start.elapsed()));
    diag.progress(&format_progress(1, 1, &msg, is_terminal()));

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
    let is_term = is_terminal();

    diag_info!(diag, "{} (threads: {})", name, workers);

    let counter = AtomicUsize::new(0);
    let prev_value = AtomicUsize::new(0);

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
                let current = counter.fetch_add(1, Ordering::Relaxed) + 1;
                // Only print if value changed (avoid duplicate prints)
                let prev = prev_value.swap(current, Ordering::Relaxed);
                if current != prev {
                    let msg = format_elapsed(start.elapsed());
                    diag.progress(&format_progress(current, total, &msg, is_term));
                }
                result
            })
            .collect()
    });

    let msg = format!("Finished in {}", format_elapsed(start.elapsed()));
    diag.progress(&format_progress(total, total, &msg, is_term));

    results
}
