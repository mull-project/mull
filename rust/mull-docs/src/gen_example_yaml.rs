use mull_core::config::MullConfigSpec;
use schemars::schema_for;
use serde_json::Value;
use std::fs;
use std::path::Path;

pub fn generate(output_dir: &Path) {
    let schema = schema_for!(MullConfigSpec);
    let schema_value = serde_json::to_value(&schema).expect("failed to serialize schema");
    let defaults = MullConfigSpec::default();
    let default_value = serde_yaml::to_value(&defaults).expect("failed to serialize defaults");

    let mut yaml = String::new();

    if let Some(props) = get_properties(&schema_value) {
        for (key, prop_schema) in props {
            let description = get_description(prop_schema, &schema_value);

            if let Some(value) = default_value.get(key) {
                if let serde_yaml::Value::Mapping(map) = value {
                    if !description.is_empty() {
                        yaml.push_str(&format!("# {}\n", description));
                    }
                    yaml.push_str(&format!("{}:\n", key));

                    let nested_props = get_nested_properties(prop_schema, &schema_value);

                    for (nested_key, nested_value) in map {
                        if let serde_yaml::Value::String(k) = nested_key {
                            let nested_desc = nested_props
                                .and_then(|p| p.get(k.as_str()))
                                .map(|s| get_description(s, &schema_value))
                                .unwrap_or_default();

                            if !nested_desc.is_empty() {
                                yaml.push_str(&format!("  # {}\n", nested_desc));
                            }

                            let value_str = if key == "parallelization"
                                && (k == "workers" || k == "executionWorkers")
                            {
                                "$nproc".to_string()
                            } else {
                                format_yaml_value(nested_value)
                            };

                            yaml.push_str(&format!("  {}: {}\n", k, value_str));
                        }
                    }
                } else {
                    if !description.is_empty() {
                        yaml.push_str(&format!("# {}\n", description));
                    }
                    yaml.push_str(&format!("{}: {}\n", key, format_yaml_value(value)));
                }
            }
        }
    }

    let mut out = String::new();
    out.push_str(".. code-block:: yaml\n\n");
    for line in yaml.lines() {
        out.push_str(&format!("   {}\n", line));
    }

    let path = output_dir.join("mull-config-example.rst");
    fs::write(&path, out).expect("failed to write config example RST");
    eprintln!("Generated {}", path.display());
}

fn resolve_ref<'a>(schema: &'a Value, root: &'a Value) -> Option<&'a Value> {
    if let Some(ref_path) = schema.get("$ref").and_then(|r| r.as_str()) {
        let def_name = ref_path
            .strip_prefix("#/$defs/")
            .or_else(|| ref_path.strip_prefix("#/definitions/"))?;
        return root
            .get("$defs")
            .or_else(|| root.get("definitions"))
            .and_then(|d| d.get(def_name));
    }
    if let Some(Value::Array(all_of)) = schema.get("allOf") {
        for inner in all_of {
            if let Some(ref_path) = inner.get("$ref").and_then(|r| r.as_str()) {
                let def_name = ref_path
                    .strip_prefix("#/$defs/")
                    .or_else(|| ref_path.strip_prefix("#/definitions/"))?;
                return root
                    .get("$defs")
                    .or_else(|| root.get("definitions"))
                    .and_then(|d| d.get(def_name));
            }
        }
    }
    Some(schema)
}

fn get_description(schema: &Value, root: &Value) -> String {
    let resolved = resolve_ref(schema, root).unwrap_or(schema);
    resolved
        .get("description")
        .and_then(|d| d.as_str())
        .unwrap_or("")
        .to_string()
}

fn get_properties(schema: &Value) -> Option<&serde_json::Map<String, Value>> {
    schema.get("properties")?.as_object()
}

fn get_nested_properties<'a>(
    schema: &'a Value,
    root: &'a Value,
) -> Option<&'a serde_json::Map<String, Value>> {
    let resolved = resolve_ref(schema, root)?;
    resolved.get("properties")?.as_object()
}

fn format_yaml_value(value: &serde_yaml::Value) -> String {
    match value {
        serde_yaml::Value::Bool(b) => b.to_string(),
        serde_yaml::Value::Number(n) => n.to_string(),
        serde_yaml::Value::String(s) if s.is_empty() => "\"\"".to_string(),
        serde_yaml::Value::String(s) => format!("\"{}\"", s),
        serde_yaml::Value::Sequence(seq) if seq.is_empty() => "[]".to_string(),
        serde_yaml::Value::Sequence(_) => {
            serde_yaml::to_string(value).unwrap_or_else(|_| "[]".to_string())
        }
        serde_yaml::Value::Mapping(_) => {
            serde_yaml::to_string(value).unwrap_or_else(|_| "{}".to_string())
        }
        serde_yaml::Value::Null => "null".to_string(),
        serde_yaml::Value::Tagged(t) => format_yaml_value(&t.value),
    }
}
