use clap::{CommandFactory, FromArgMatches};
use mull_core::config::{MullConfigSpec, ReporterCli};
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{
    diag_debug, diag_info, diag_warning, get_config_path, long_version_string, normalize_args,
};
use mull_reporters::{create_reporters, ReporterConfig};
use mull_state::{restore_execution_state, ExecutionState, ExecutionStatus};
use std::fs;
use std::process::exit;
use std::time::Instant;

fn init_diagnostics(diag: &MullDiagnostics, config: &MullConfigSpec) {
    if config.debug_enabled {
        diag.enable_debug_mode();
        diag_debug!(
            diag,
            "Diagnostics: Debug Mode enabled. Debug-level messages will be printed."
        );
    }
    if config.strict {
        diag.enable_strict_mode();
        diag_info!(
            diag,
            "Diagnostics: Strict Mode enabled. Warning messages will be treated as fatal errors."
        );
    }
    if config.silent {
        diag.make_silent();
    }
    if config.quiet {
        diag.make_quiet();
    }
}

fn parse_yaml(yaml: &str) -> Result<MullConfigSpec, String> {
    let parsed: MullConfigSpec =
        serde_yaml::from_str(yaml).map_err(|e| format!("Failed to parse YAML: {}", e))?;
    Ok(parsed)
}

fn load_yaml_config(config_path: &str) -> Result<MullConfigSpec, String> {
    let contents =
        fs::read_to_string(config_path).map_err(|e| format!("Cannot read config: {}", e))?;
    parse_yaml(&contents)
}

fn create_config(diag: &MullDiagnostics) -> MullConfigSpec {
    match get_config_path() {
        Some(path) => match load_yaml_config(&path) {
            Ok(config) => {
                diag_info!(diag, "Using config {}", path);
                config
            }
            Err(e) => {
                diag_warning!(
                    diag,
                    "Failed to read config: {}: {}. Using some defaults.",
                    path,
                    e
                );
                MullConfigSpec::default()
            }
        },
        None => {
            diag_warning!(
                diag,
                "Mull cannot find config (mull.yml). Using some defaults."
            );
            MullConfigSpec::default()
        }
    }
}

fn override_config(cli: &ReporterCli, yaml_config: &mut MullConfigSpec) {
    if cli.shared.debug {
        yaml_config.debug_enabled = true;
    }
    if cli.shared.no_test_output || cli.shared.no_output {
        yaml_config.capture_test_output = false;
    }
    if cli.shared.no_mutant_output || cli.shared.no_output {
        yaml_config.capture_mutant_output = false;
    }
}

fn init_reporter_cli(
    args: Vec<String>,
    llvm_version: String,
) -> (MullConfigSpec, ReporterCli, MullDiagnostics) {
    let args = normalize_args(&args);
    let cmd = ReporterCli::command().long_version(long_version_string(&llvm_version));
    let matches = cmd.get_matches_from(&args);
    let cli = ReporterCli::from_arg_matches(&matches).unwrap();

    let diag = MullDiagnostics::new();
    let mut yaml_config = create_config(&diag);
    // defaults to true, but overriding this for CLI tools
    yaml_config.quiet = false;
    override_config(&cli, &mut yaml_config);
    init_diagnostics(&diag, &yaml_config);

    (yaml_config, cli, diag)
}

fn report_mutation_score(
    diag: &MullDiagnostics,
    state: &ExecutionState,
    allow_survived: bool,
    threshold: u32,
) -> bool {
    assert!(!state.execution_results.is_empty());
    let mut killed = 0;
    let mut survived = 0;
    for (_, result) in &state.execution_results {
        if result.status != ExecutionStatus::Passed && result.status != ExecutionStatus::NotCovered
        {
            killed += 1;
        }
        if result.status == ExecutionStatus::Passed {
            survived += 1;
        }
    }

    let total = state.execution_results.len();
    let score = killed * 100 / total;
    diag_info!(diag, "Mutation score: {}%", score);
    if survived != 0 {
        diag_info!(diag, "Surviving mutants: {}", survived);
        if !allow_survived && threshold > score as u32 {
            return true;
        }
    }

    false
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let llvm_version = env!("MULL_LLVM_VERSION");
    let (_config, cli, diag) = init_reporter_cli(args, llvm_version.into());

    let start = Instant::now();

    let exec_state = restore_execution_state(&diag, &cli.sqlite_report);
    let should_fail = if exec_state.execution_results.is_empty() {
        diag_info!(diag, "No mutants found. Mutation score: infinitely high");
        false
    } else {
        let reporter_config = ReporterConfig {
            reporters: cli.shared.reporters,
            report_dir: cli.shared.report_dir,
            report_name: cli.shared.report_name.unwrap_or_default(),
            ide_reporter_show_killed: cli.shared.ide_reporter_show_killed,
            report_patch_base: cli.shared.report_patch_base,
        };
        let reporters = create_reporters(&reporter_config);
        for reporter in &reporters {
            reporter.report(&diag, &exec_state);
        }
        report_mutation_score(
            &diag,
            &exec_state,
            cli.shared.allow_surviving,
            cli.shared.mutation_score_threshold,
        )
    };

    diag_info!(
        diag,
        "Total reporting time: {:.3}ms",
        start.elapsed().as_secs_f64() * 1000.0
    );
    if should_fail {
        exit(1);
    }
}
