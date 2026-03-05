mod dynamic_libraries;
mod init_cli;
mod mutant_extractor;
mod reporting;
mod runner;

use crate::runner::run_program;
use dynamic_libraries::{get_dynamic_library_dependencies, resolve_libraries};
use mull_core::{config::RunnerCli, diag_debug, diag_error, diagnostics::MullDiagnostics};
use mull_filters::{has_coverage, FilterChain, FilterChainConfig};
use mull_state::{ExecutionResult, ExecutionState, ExecutionStatus, Mutant};
use mull_tasks::{run_parallel_task, run_task};
use mutant_extractor::extract_mutants;
use std::path::Path;
use std::time::{Duration, Instant};

use crate::reporting::show_report;

fn validate_input_file(diag: &MullDiagnostics, input_file: &str) -> Option<String> {
    let path = Path::new(input_file);
    if !path.exists() {
        diag_error!(
            diag,
            "The provided path to an executable program is not valid: {}",
            input_file
        );
    }

    match path.canonicalize() {
        Ok(p) => Some(p.to_string_lossy().into_owned()),
        Err(e) => {
            diag_error!(diag, "Cannot resolve path '{}': {}", input_file, e);
        }
    }
}

/// Create a unique temporary file path
fn temp_file_path(prefix: &str, suffix: &str) -> String {
    use std::time::{SystemTime, UNIX_EPOCH};
    let timestamp = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_nanos())
        .unwrap_or(0);
    let pid = std::process::id();
    format!(
        "{}/mull-{}-{}-{}.{}",
        std::env::temp_dir().to_string_lossy(),
        prefix,
        pid,
        timestamp,
        suffix
    )
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let llvm_version = env!("MULL_LLVM_VERSION");
    let (config, cli, diag) = init_cli::init_cli::<RunnerCli>(args, llvm_version.into());

    let start = Instant::now();

    // Validate input file
    let input_file = match validate_input_file(&diag, &cli.input_file) {
        Some(f) => f,
        None => {
            std::process::exit(1);
        }
    };

    // Collect mutant holders (executable + dynamic libraries)
    let mut mutant_holders = vec![input_file.clone()];

    // Find and resolve dynamic library dependencies
    let dependencies = get_dynamic_library_dependencies(&diag, &input_file);
    diag_debug!(
        diag,
        "Found {} dynamic library dependencies",
        dependencies.len()
    );

    let resolved = resolve_libraries(&diag, &dependencies, &cli.ld_search_paths);
    diag_debug!(diag, "Resolved {} libraries with mutants", resolved.len());
    mutant_holders.extend(resolved);

    // Extract mutants from all holders
    let mutants = extract_mutants(&diag, &mutant_holders);
    diag_debug!(diag, "Found {} mutant(s)", mutants.len());

    let executable = cli.test_program.as_ref().unwrap_or(&input_file);
    let args = &cli.runner_args;
    let timeout_ms = cli.timeout.unwrap_or(config.timeout) as u64;
    let workers = cli
        .workers
        .unwrap_or(config.parallelization.execution_workers) as usize;

    // Check for coverage instrumentation and set up coverage collection
    let coverage_info = cli.coverage_info.clone().unwrap_or_default();
    let collect_coverage = coverage_info.is_empty() && has_coverage(&input_file);
    let raw_coverage_path = if collect_coverage {
        let raw = temp_file_path("coverage", "profraw");
        if config.debug.coverage || cli.debug_coverage {
            diag_debug!(diag, "coverageFile: {}", raw);
        }
        Some(raw)
    } else {
        None
    };

    // Build environment for warmup run
    let mut warmup_env: Vec<(&str, &str)> = Vec::new();
    let raw_coverage_path_ref = raw_coverage_path.as_deref();
    if let Some(ref raw_path) = raw_coverage_path_ref {
        warmup_env.push(("LLVM_PROFILE_FILE", raw_path));
    }

    let warmup = run_task(&diag, "Warm up run", || {
        run_program(
            executable,
            args,
            &warmup_env,
            Duration::from_millis(timeout_ms),
            config.capture_test_output,
        )
    });
    warmup.error_on_failure(&diag, "Original test failed (warmup run)");

    // Determine coverage profile path
    // llvm_profparser can parse both raw (.profraw) and indexed (.profdata) formats
    let coverage_profile_path = if let Some(raw_path) = &raw_coverage_path {
        if std::path::Path::new(raw_path).exists() {
            Some(raw_path.clone())
        } else {
            if config.debug.coverage {
                diag_debug!(diag, "Raw coverage file not created: {}", raw_path);
            }
            None
        }
    } else if !coverage_info.is_empty() {
        Some(coverage_info)
    } else {
        None
    };

    // Build and apply filter chain
    let filter_chain = FilterChain::from_config(
        &diag,
        FilterChainConfig {
            coverage_profile_path: coverage_profile_path.as_deref(),
            object_files: &mutant_holders,
            include_not_covered: config.include_not_covered || cli.include_not_covered,
            debug_coverage: config.debug.coverage || cli.debug_coverage,
            git_diff_ref: &config.git_diff_ref,
            git_project_root: &config.git_project_root,
            debug_git_diff: config.debug.git_diff,
        },
    );

    let debug_filters = config.debug.filters;
    let filter_result = filter_chain.filter(mutants, &diag, debug_filters, workers);

    let filtered_mutants = filter_result.to_execute;
    let not_covered_mutants: Vec<Mutant> = filter_result
        .to_report
        .into_iter()
        .map(|(m, _status)| m)
        .collect();

    if config.debug.coverage {
        diag_debug!(
            diag,
            "{} mutant(s) to execute, {} mutant(s) to report only",
            filtered_mutants.len(),
            not_covered_mutants.len()
        );
    }

    // Dry run mode - assign DryRun status to all mutants without executing
    if cli.dry_run || config.dry_run_enabled {
        let all_mutants: Vec<Mutant> = filtered_mutants
            .into_iter()
            .chain(not_covered_mutants.into_iter())
            .collect();
        let mut state = ExecutionState::default();
        run_task(&diag, "Dry run", || {
            for mutant in all_mutants {
                state.execution_results.insert(
                    mutant,
                    ExecutionResult {
                        status: ExecutionStatus::DryRun,
                        exit_status: 0,
                        running_time: 0,
                        stdout_output: String::new(),
                        stderr_output: String::new(),
                    },
                );
            }
        });
        state
            .info
            .insert("Mull Version".into(), env!("MULL_VERSION").into());
        state
            .info
            .insert("LLVM Version".into(), llvm_version.into());
        show_report(&diag, &state, cli.shared, &start, "dry run");
        return;
    }

    // If no mutants (covered or not-covered), show report and exit
    if filtered_mutants.is_empty() && not_covered_mutants.is_empty() {
        let state = ExecutionState::default();
        show_report(&diag, &state, cli.shared, &start, "execution");
        return;
    }

    // If no covered mutants, skip running tests but still report not-covered
    if filtered_mutants.is_empty() {
        let mut state = ExecutionState::default();
        for mutant in not_covered_mutants {
            state.execution_results.insert(
                mutant,
                ExecutionResult {
                    status: ExecutionStatus::NotCovered,
                    exit_status: 0,
                    running_time: 0,
                    stdout_output: String::new(),
                    stderr_output: String::new(),
                },
            );
        }
        state
            .info
            .insert("Mull Version".into(), env!("MULL_VERSION").into());
        state
            .info
            .insert("LLVM Version".into(), llvm_version.into());
        show_report(&diag, &state, cli.shared, &start, "execution");
        return;
    }

    let baseline = run_task(&diag, "Baseline run", || {
        run_program(
            executable,
            args,
            &[],
            Duration::from_millis(timeout_ms),
            config.capture_test_output,
        )
    });
    baseline.error_on_failure(&diag, "Original test failed (baseline run)");

    let mutant_timeout_ms = std::cmp::max(30, baseline.running_time as u64 * 10);
    let mutant_timeout = Duration::from_millis(mutant_timeout_ms);
    diag_debug!(
        diag,
        "Baseline: {}ms, mutant timeout: {}ms",
        baseline.running_time,
        mutant_timeout_ms
    );
    let total = filtered_mutants.len();
    let results: Vec<(Mutant, ExecutionResult)> = run_parallel_task(
        &diag,
        "Running mutants",
        workers,
        filtered_mutants,
        |idx, mutant| {
            diag_debug!(
                diag,
                "[{}/{}] Running: {}={} {} {}",
                idx + 1,
                total,
                mutant.identifier,
                "1",
                executable,
                args.join(" ")
            );
            let result = run_program(
                executable,
                args,
                &[(&mutant.identifier, "1")],
                mutant_timeout,
                config.capture_mutant_output,
            );
            diag_debug!(
                diag,
                "[{}/{}] {} -> {}, {}ms",
                idx + 1,
                total,
                mutant.identifier,
                result.status,
                result.running_time
            );
            (mutant, result)
        },
    );

    let mut state = ExecutionState::default();
    for (mutant, result) in results.into_iter() {
        state.execution_results.insert(mutant, result);
    }
    // Add not-covered mutants with NotCovered status
    for mutant in not_covered_mutants {
        state.execution_results.insert(
            mutant,
            ExecutionResult {
                status: ExecutionStatus::NotCovered,
                exit_status: 0,
                running_time: 0,
                stdout_output: String::new(),
                stderr_output: String::new(),
            },
        );
    }
    state
        .info
        .insert("Mull Version".into(), env!("MULL_VERSION").into());
    state
        .info
        .insert("LLVM Version".into(), llvm_version.into());

    // Cleanup temp coverage file
    if let Some(raw_path) = &raw_coverage_path {
        let _ = std::fs::remove_file(raw_path);
    }

    show_report(&diag, &state, cli.shared, &start, "execution");
}
