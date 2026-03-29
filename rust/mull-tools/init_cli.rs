use clap::{CommandFactory, FromArgMatches};
use mull_core::config::{HasSharedCli, MullConfigSpec, SharedCli};
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_debug, diag_info, diag_warning, get_config_path};
use std::fs;

fn normalize_args(args: &[String]) -> Vec<String> {
    let mut out = Vec::with_capacity(args.len());
    let mut passthrough = false;
    for (i, arg) in args.iter().enumerate() {
        if i == 0 || passthrough {
            out.push(arg.clone());
            continue;
        }
        if arg == "--" {
            passthrough = true;
            out.push(arg.clone());
            continue;
        }
        // Single-dash with >1 char after it: convert to double-dash.
        // e.g. "-reporters" -> "--reporters", "-timeout=5000" -> "--timeout=5000"
        if arg.starts_with('-') && !arg.starts_with("--") && arg.len() > 2 {
            out.push(format!("-{}", arg));
        } else {
            out.push(arg.clone());
        }
    }
    out
}

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

fn create_config(diag: &MullDiagnostics) -> (MullConfigSpec, Option<String>) {
    match get_config_path() {
        Some(path) => match load_yaml_config(&path) {
            Ok(config) => (config, Some(path.into())),
            Err(e) => {
                diag_warning!(
                    diag,
                    "Failed to read config: {}: {}. Using some defaults.",
                    path,
                    e
                );
                (MullConfigSpec::default(), None)
            }
        },
        None => {
            diag_warning!(
                diag,
                "Mull cannot find config (mull.yml). Using some defaults."
            );
            (MullConfigSpec::default(), None)
        }
    }
}

fn override_config(shared: &SharedCli, yaml_config: &mut MullConfigSpec) {
    if shared.debug {
        yaml_config.debug_enabled = true;
    }
    if shared.no_test_output || shared.no_output {
        yaml_config.capture_test_output = false;
    }
    if shared.no_mutant_output || shared.no_output {
        yaml_config.capture_mutant_output = false;
    }
}

fn long_version_string(mull_version: &str, llvm_version: &str) -> &'static str {
    let s = format!(
        "{}\n\n\
         Mull: Practical mutation testing and fault injection for C and C++\n\
         Home: https://github.com/mull-project/mull\n\
         Docs: https://mull.readthedocs.io\n\
         Support: https://mull.readthedocs.io/en/latest/Support.html\n\
         LLVM: {}",
        mull_version, llvm_version
    );
    // Leak is fine: this runs once per CLI invocation then the process exits.
    Box::leak(s.into_boxed_str())
}

pub fn init_cli<C>(
    args: Vec<String>,
    mull_version: String,
    llvm_version: String,
) -> (MullConfigSpec, C, MullDiagnostics)
where
    C: CommandFactory + FromArgMatches + HasSharedCli,
{
    let args = normalize_args(&args);
    let cmd = C::command().long_version(long_version_string(&mull_version, &llvm_version));
    let matches = cmd.get_matches_from(&args);
    let cli = C::from_arg_matches(&matches).unwrap();

    let diag = MullDiagnostics::new();
    let (mut yaml_config, maybe_path) = create_config(&diag);
    yaml_config.quiet = false;

    override_config(cli.shared(), &mut yaml_config);
    init_diagnostics(&diag, &yaml_config);
    if let Some(path) = maybe_path {
        diag_info!(diag, "Using config {}", path);
    }

    (yaml_config, cli, diag)
}
