mod dynamic_libraries;
mod init_cli;
mod mutant_extractor;
mod plugins;
mod reporting;
mod runner;

use crate::runner::{kill_programs_on_interrupt, run_program};
use dynamic_libraries::find_dynamic_libraries;
use mull_cache::{default_cache_path, fingerprint, Cache, RunIdentity};
use mull_core::{
    config::RunnerCli, diag_debug, diag_error, diag_info, diag_warning,
    diagnostics::MullDiagnostics,
};
use mull_filters::{has_coverage, FilterChain, FilterChainConfig};
use mull_state::{ExecutionResult, ExecutionState, ExecutionStatus, Mutant};
use mull_tasks::{run_parallel_task, run_task};
use mutant_extractor::extract_mutants;
use std::path::{Path, PathBuf};
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

fn canonical(path: &str) -> String {
    std::fs::canonicalize(path)
        .map(|p| p.to_string_lossy().into_owned())
        .unwrap_or_else(|_| path.to_string())
}

/// Failing to open the default cache is not worth a warning, an explicit one is
fn open_cache(
    diag: &MullDiagnostics,
    cli: &RunnerCli,
    mutant_holders: &[String],
    test_program: &str,
) -> Option<Cache> {
    if cli.no_cache {
        return None;
    }
    let test_program = canonical(test_program);
    let identity = RunIdentity {
        executable: mutant_holders[0].clone(),
        test_program: test_program.clone(),
        args: cli.runner_args.clone(),
        dylibs: mutant_holders[1..].to_vec(),
        working_directory: std::env::current_dir()
            .map(|p| p.to_string_lossy().into_owned())
            .unwrap_or_default(),
    };
    let explicit = cli.cache_path.is_some();
    let path = match &cli.cache_path {
        Some(path) => PathBuf::from(path),
        None => {
            let mut files: Vec<&str> = mutant_holders.iter().map(String::as_str).collect();
            if Path::new(&test_program).is_file() && !files.contains(&test_program.as_str()) {
                files.push(&test_program);
            }
            match fingerprint(&files) {
                Ok(fp) => default_cache_path(&identity, &fp),
                Err(e) => {
                    diag_debug!(diag, "No cache: cannot fingerprint the run: {}", e);
                    return None;
                }
            }
        }
    };
    let opened = path
        .parent()
        .filter(|dir| !dir.as_os_str().is_empty())
        .map_or(Ok(()), std::fs::create_dir_all)
        .map_err(|e| e.to_string())
        .and_then(|_| {
            Cache::open(
                &path,
                &identity.key(),
                Duration::from_millis(cli.shared.sqlite_busy_timeout as u64),
            )
            .map_err(|e| e.to_string())
        });
    match opened {
        Ok(cache) => {
            diag_info!(diag, "Using cache: {}", path.display());
            Some(cache)
        }
        Err(e) => {
            let message = format!("Cannot open cache '{}': {}", path.display(), e);
            if explicit {
                diag_warning!(diag, "{}", message);
            } else {
                diag_debug!(diag, "{}", message);
            }
            None
        }
    }
}

struct MutantRun<'a> {
    executable: &'a str,
    args: &'a [String],
    timeout_ms: u64,
    minimum_timeout_ms: u64,
    capture_test_output: bool,
    capture_mutant_output: bool,
    workers: usize,
    cache: Option<&'a Cache>,
}

fn run_mutants(
    diag: &MullDiagnostics,
    run: &MutantRun,
    mutants: Vec<Mutant>,
) -> Vec<(Mutant, ExecutionResult)> {
    let MutantRun {
        executable, args, ..
    } = *run;
    diag_debug!(diag, "Running mutants for: {}", executable);

    let baseline = run_task(diag, "Baseline run", || {
        run_program(
            executable,
            args,
            &[],
            Duration::from_millis(run.timeout_ms),
            run.capture_test_output,
        )
        .unwrap_or_else(|e| e.exit(diag, executable))
    });
    baseline.error_on_failure(diag, "Original test failed (baseline run)");

    let baseline_timeout = baseline.running_time as u64 * 10;
    let mutant_timeout_ms = std::cmp::max(baseline_timeout, run.minimum_timeout_ms);
    let mutant_timeout = Duration::from_millis(mutant_timeout_ms);
    diag_debug!(
        diag,
        "Baseline: {}ms, mutant timeout: {}ms",
        baseline.running_time,
        mutant_timeout_ms
    );
    let total = mutants.len();
    run_parallel_task(
        diag,
        "Running mutants",
        run.workers,
        mutants,
        |idx, mutant| {
            // A concurrent runner sharing the cache may have killed it meanwhile
            if let Some(cache) = run.cache {
                match cache.foreign_kill(&mutant) {
                    Ok(Some(result)) => return (mutant, result),
                    Ok(None) => {}
                    Err(e) => diag_warning!(diag, "Cannot read cache: {}", e),
                }
            }
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
                run.capture_mutant_output,
            )
            .unwrap_or_else(|e| e.exit(diag, executable));
            diag_debug!(
                diag,
                "[{}/{}] {} -> {}, {}ms",
                idx + 1,
                total,
                mutant.identifier,
                result.status,
                result.running_time
            );
            if let Some(cache) = run.cache {
                if let Err(e) = cache.store(&mutant, &result) {
                    diag_warning!(diag, "Cannot write cache: {}", e);
                }
            }
            (mutant, result)
        },
    )
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let mull_version = env!("MULL_VERSION");
    let (config, cli, diag) = init_cli::init_cli::<RunnerCli>(args, mull_version.into());

    if let Err(e) = kill_programs_on_interrupt() {
        diag_warning!(diag, "Cannot install the interrupt handler: {}", e);
    }

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
    let resolved = find_dynamic_libraries(&diag, &input_file, &cli.ld_search_paths);
    diag_debug!(diag, "Resolved {} dynamic libraries", resolved.len());
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
    if let Some(raw_path) = raw_coverage_path_ref {
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
        .unwrap_or_else(|e| e.exit(&diag, executable))
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
            git_diff_paths: &config.git_diff_paths,
            debug_git_diff: config.debug.git_diff,
            mutation_ranges: &config.mutation_ranges,
            // disabled as it's likely reapply the same filter as during the compilation
            enable_manual_filter: false,
            include_paths: &config.include_paths,
            exclude_paths: &config.exclude_paths,
            debug_filepath: config.debug.filters,
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

    let dry_run = cli.dry_run || config.dry_run_enabled;
    let mut state = ExecutionState::default();
    if dry_run {
        run_task(&diag, "Dry run", || {
            for mutant in filtered_mutants.into_iter().chain(not_covered_mutants) {
                state.execution_results.insert(
                    mutant,
                    ExecutionResult::with_status(ExecutionStatus::DryRun),
                );
            }
        });
    } else {
        if !filtered_mutants.is_empty() {
            let cache = open_cache(&diag, &cli, &mutant_holders, executable);
            let mut cached = match cache.as_ref().map(Cache::load) {
                Some(Ok(cached)) => cached,
                Some(Err(e)) => {
                    diag_warning!(diag, "Cannot read cache: {}", e);
                    Default::default()
                }
                None => Default::default(),
            };
            let mut pending = Vec::new();
            for mutant in filtered_mutants {
                let key = (mutant.identifier.clone(), mutant.replacement.clone());
                match cached.remove(&key) {
                    Some(result) => {
                        state.execution_results.insert(mutant, result);
                    }
                    None => pending.push(mutant),
                }
            }
            if !state.execution_results.is_empty() {
                diag_info!(
                    diag,
                    "Reusing {} cached result(s)",
                    state.execution_results.len()
                );
            }
            if !pending.is_empty() {
                let run = MutantRun {
                    executable,
                    args,
                    timeout_ms,
                    minimum_timeout_ms: cli.minimum_timeout.unwrap_or(30),
                    capture_test_output: config.capture_test_output,
                    capture_mutant_output: config.capture_mutant_output,
                    workers,
                    cache: cache.as_ref(),
                };
                state
                    .execution_results
                    .extend(run_mutants(&diag, &run, pending));
            }
        }
        for mutant in not_covered_mutants {
            state.execution_results.insert(
                mutant,
                ExecutionResult::with_status(ExecutionStatus::NotCovered),
            );
        }
    }
    if !state.execution_results.is_empty() {
        state
            .info
            .insert("Mull Version".into(), mull_version.into());
    }

    if let Some(raw_path) = &raw_coverage_path {
        let _ = std::fs::remove_file(raw_path);
    }

    let label = if dry_run { "dry run" } else { "execution" };
    show_report(&diag, &state, cli.shared, &start, label);
}
