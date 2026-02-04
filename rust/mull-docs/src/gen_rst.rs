use mull_config::config::{OptionMeta, option_metadata};
use std::fs;
use std::path::Path;

pub fn generate(output_dir: &Path) {
    let metadata = option_metadata();

    generate_cli_rst(
        &metadata,
        |o| o.scope.in_runner(),
        output_dir,
        "mull-runner-cli-options.rst",
    );
    generate_cli_rst(
        &metadata,
        |o| o.scope.in_reporter(),
        output_dir,
        "mull-reporter-cli-options.rst",
    );
    generate_config_reference(&metadata, output_dir);
}

/// Generate CLI options RST in the existing `--flag value\t\tDescription` format.
fn generate_cli_rst(
    metadata: &[OptionMeta],
    filter: impl Fn(&OptionMeta) -> bool,
    output_dir: &Path,
    filename: &str,
) {
    let mut out = String::new();

    for opt in metadata {
        let long = match opt.cli_long {
            Some(l) => l,
            None => continue,
        };
        if !filter(opt) {
            continue;
        }

        if opt.value_hint.is_empty() {
            out.push_str(&format!("--{}\t\t{}\n", long, opt.help));
        } else {
            out.push_str(&format!("--{} {}\t\t{}\n", long, opt.value_hint, opt.help));
        }

        if !opt.extra.is_empty() {
            out.push_str(opt.extra);
        }

        out.push('\n');
    }

    let out = out.trim_end_matches('\n').to_string() + "\n";
    let path = output_dir.join(filename);
    fs::write(&path, out).expect("failed to write RST");
    eprintln!("Generated {}", path.display());
}

/// Generate YAML config reference RST for the docs site.
fn generate_config_reference(metadata: &[OptionMeta], output_dir: &Path) {
    let mut out = String::new();

    // Collect unique sections for YAML options
    let mut sections: Vec<&str> = Vec::new();
    for opt in metadata {
        if opt.yaml_key.is_some() && !sections.contains(&opt.section) {
            sections.push(opt.section);
        }
    }

    for section in sections {
        let opts: Vec<_> = metadata
            .iter()
            .filter(|o| o.section == section && o.yaml_key.is_some())
            .collect();
        if opts.is_empty() {
            continue;
        }

        out.push_str(&format!("{}\n", section));
        let underline: String = std::iter::repeat('-').take(section.len()).collect();
        out.push_str(&underline);
        out.push_str("\n\n");

        for opt in &opts {
            let yaml_key = opt.yaml_key.unwrap();
            out.push_str(&format!("``{}``\n", yaml_key));
            out.push_str(&format!("   {}\n\n", opt.help));

            if let Some(cli) = opt.cli_long {
                out.push_str(&format!("   CLI equivalent: ``--{}``\n\n", cli));
            }
        }
    }

    let out = out.trim_end_matches('\n').to_string() + "\n";
    let path = output_dir.join("mull-config-reference.rst");
    fs::write(&path, out).expect("failed to write config RST");
    eprintln!("Generated {}", path.display());
}
