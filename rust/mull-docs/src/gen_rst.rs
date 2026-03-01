use clap::{Command, CommandFactory};
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
fn generate_cli_rst(mut cmd: Command, output_dir: &Path, filename: &str) {
    // Use clap's built-in help rendering which includes all details
    // (possible values, defaults, etc.)
    let help = cmd.render_long_help();
    let help_str = help.to_string();

    // Wrap in RST code block for proper formatting
    // Trim trailing whitespace from each line to satisfy pre-commit hooks
    let out = format!(
        ".. code-block:: text\n\n{}\n",
        help_str
            .lines()
            .map(|line| {
                let trimmed = line.trim_end();
                if trimmed.is_empty() {
                    String::new()
                } else {
                    format!("   {}", trimmed)
                }
            })
            .collect::<Vec<_>>()
            .join("\n")
            .trim_end()
    );

    let path = output_dir.join(filename);
    fs::write(&path, &out).expect("failed to write RST");
    eprintln!("Generated {}", path.display());
}
