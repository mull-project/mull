use mull_config::config::option_metadata;
use std::fs;
use std::path::Path;

pub fn generate(output_dir: &Path) {
    let metadata = option_metadata();
    let mut out = String::new();

    out.push_str("# Mull configuration file\n");
    out.push_str("# See https://mull.readthedocs.io for documentation\n\n");

    // Collect unique sections in order, but only for options with YAML keys
    let mut sections: Vec<&str> = Vec::new();
    for opt in &metadata {
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

        out.push_str(&format!("# {}\n", section));

        // Determine if this section is nested (e.g. "debug.printIr")
        let is_nested = opts
            .first()
            .and_then(|o| o.yaml_key)
            .map(|k| k.contains('.'))
            .unwrap_or(false);

        if is_nested {
            let parent = opts[0].yaml_key.unwrap().split('.').next().unwrap();
            out.push_str(&format!("# {}:\n", parent));

            for opt in &opts {
                let yaml_key = opt.yaml_key.unwrap();
                let child_key = yaml_key.split('.').nth(1).unwrap_or(yaml_key);
                out.push_str(&format!("#   # {}\n", opt.help));
                out.push_str(&format!("#   {}: {}\n", child_key, opt.default_display));
            }
        } else {
            for opt in &opts {
                let yaml_key = opt.yaml_key.unwrap();
                out.push_str(&format!("# {}\n", opt.help));
                out.push_str(&format!("# {}: {}\n", yaml_key, opt.default_display));
            }
        }
        out.push('\n');
    }

    let path = output_dir.join("mull.yml.example");
    fs::write(&path, out).expect("failed to write example YAML");
    eprintln!("Generated {}", path.display());
}
