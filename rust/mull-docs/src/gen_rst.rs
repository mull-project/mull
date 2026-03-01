use clap::{Arg, ArgAction, Command, CommandFactory};
use mull_core::config::{ReporterCli, RunnerCli};
use std::fs;
use std::path::Path;

pub fn generate(output_dir: &Path) {
    generate_cli_rst(
        RunnerCli::command(),
        output_dir,
        "mull-runner-cli-options.rst",
    );
    generate_cli_rst(
        ReporterCli::command(),
        output_dir,
        "mull-reporter-cli-options.rst",
    );
}

/// Generate CLI options RST from clap Command.
fn generate_cli_rst(cmd: Command, output_dir: &Path, filename: &str) {
    let mut out = String::new();

    for arg in cmd.get_arguments() {
        // Skip help and version
        if arg.get_id() == "help" || arg.get_id() == "version" {
            continue;
        }

        if let Some(long) = arg.get_long() {
            let help = arg.get_help().map(|h| h.to_string()).unwrap_or_default();

            // Don't show value name for boolean flags
            if is_bool_flag(arg) {
                out.push_str(&format!("--{}\t\t{}\n", long, help));
            } else {
                let value_name = get_value_name(arg);
                if value_name.is_empty() {
                    out.push_str(&format!("--{}\t\t{}\n", long, help));
                } else {
                    out.push_str(&format!("--{} {}\t\t{}\n", long, value_name, help));
                }
            }
            out.push('\n');
        }
    }

    // Handle positional arguments
    for arg in cmd.get_positionals() {
        let help = arg.get_help().map(|h| h.to_string()).unwrap_or_default();
        let value_name = arg
            .get_value_names()
            .and_then(|v| v.first())
            .map(|s| s.to_string())
            .unwrap_or_else(|| arg.get_id().to_string().to_uppercase());

        out.push_str(&format!("{}\t\t{}\n\n", value_name, help));
    }

    let out = out.trim_end_matches('\n').to_string() + "\n";
    let path = output_dir.join(filename);
    fs::write(&path, out).expect("failed to write RST");
    eprintln!("Generated {}", path.display());
}

fn is_bool_flag(arg: &Arg) -> bool {
    matches!(arg.get_action(), ArgAction::SetTrue | ArgAction::SetFalse)
}

fn get_value_name(arg: &Arg) -> String {
    arg.get_value_names()
        .and_then(|v| v.first())
        .map(|s| s.to_string())
        .unwrap_or_default()
}
