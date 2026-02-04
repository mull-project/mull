use mull_config::config::option_metadata;
use std::fs;
use std::path::Path;

pub fn generate(output_dir: &Path) {
    let metadata = option_metadata();

    let mut yaml = String::new();
    let mut current_prefix: Option<String> = None;

    for entry in &metadata {
        let yaml_key = match entry.yaml_key {
            Some(k) if !entry.yaml_default.is_empty() => k,
            _ => continue,
        };

        if let Some(dot_pos) = yaml_key.find('.') {
            let prefix = &yaml_key[..dot_pos];
            let subkey = &yaml_key[dot_pos + 1..];

            if current_prefix.as_deref() != Some(prefix) {
                yaml.push_str(&format!("{}:\n", prefix));
                current_prefix = Some(prefix.to_string());
            }
            yaml.push_str(&format!("  # {}\n", entry.help));
            yaml.push_str(&format!("  {}: {}\n", subkey, entry.yaml_default));
        } else {
            current_prefix = None;
            yaml.push_str(&format!("# {}\n", entry.help));
            yaml.push_str(&format!("{}: {}\n", yaml_key, entry.yaml_default));
        }
    }

    let mut out = String::new();
    out.push_str("Full configuration example\n");
    out.push_str("-------------------------\n\n");
    out.push_str(".. code-block:: yaml\n\n");
    for line in yaml.lines() {
        out.push_str(&format!("   {}\n", line));
    }

    let path = output_dir.join("mull-config-example.rst");
    fs::write(&path, out).expect("failed to write config example RST");
    eprintln!("Generated {}", path.display());
}
