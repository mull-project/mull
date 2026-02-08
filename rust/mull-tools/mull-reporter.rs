use clap::{CommandFactory, FromArgMatches};
use mull_core::config::{MullConfigSpec, ReporterCli};
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{
    diag_debug, diag_info, diag_warning, get_config_path, long_version_string, normalize_args,
};
use mull_reporters::ide_reporter::IDEReporter;
use mull_reporters::Reporter;
use mull_state::restore_execution_state;
use std::fs;

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

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let (config, cli, diag) = init_reporter_cli(args, "20".into());

    let exec_state = restore_execution_state(&diag, &cli.sqlite_report);
    let reporter = IDEReporter::new();
    reporter.report(&exec_state);
}
