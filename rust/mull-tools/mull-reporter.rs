mod init_cli;
mod reporting;

use mull_core::config::ReporterCli;
use mull_state::restore_execution_state;
use std::time::Instant;

use crate::reporting::show_report;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let llvm_version = env!("MULL_LLVM_VERSION");
    let (_config, cli, diag) = init_cli::init_cli::<ReporterCli>(args, llvm_version.into());

    let start = Instant::now();

    let exec_state = restore_execution_state(&diag, &cli.sqlite_report);
    show_report(&diag, &exec_state, cli.shared, &start, "reporting");
}
