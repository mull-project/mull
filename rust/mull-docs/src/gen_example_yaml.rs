use mull_core::config::MullConfigSpec;
use schemars::schema::{RootSchema, Schema};
use schemars::{schema_for, Map};
use std::fs;
use std::path::Path;

pub fn generate(output_dir: &Path) {
    let schema = schema_for!(MullConfigSpec);
    let defaults = MullConfigSpec::default();
    let default_value = serde_yaml::to_value(&defaults).expect("failed to serialize defaults");

    let mut yaml = String::new();

    // Get the object properties from the schema
    if let Some(obj) = &schema.schema.object {
        for (key, prop_schema) in &obj.properties {
            let description = get_description(prop_schema, &schema);

            // Get the default value for this key
            if let Some(value) = default_value.get(key) {
                // Check if this is a nested object (like parallelization or debug)
                if let serde_yaml::Value::Mapping(map) = value {
                    if !description.is_empty() {
                        yaml.push_str(&format!("# {}\n", description));
                    }
                    yaml.push_str(&format!("{}:\n", key));

                    // Get nested schema for descriptions (resolving $ref if needed)
                    let nested_props = get_nested_properties(prop_schema, &schema);

                    for (nested_key, nested_value) in map {
                        if let serde_yaml::Value::String(k) = nested_key {
                            let nested_desc = nested_props
                                .as_ref()
                                .and_then(|p| p.get(k))
                                .map(|s| get_description(s, &schema))
                                .unwrap_or_default();

                            if !nested_desc.is_empty() {
                                yaml.push_str(&format!("  # {}\n", nested_desc));
                            }

                            // Special case: parallelization workers default to nproc
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

    // Wrap in RST code block
    let mut out = String::new();
    out.push_str(".. code-block:: yaml\n\n");
    for line in yaml.lines() {
        out.push_str(&format!("   {}\n", line));
    }

    let path = output_dir.join("mull-config-example.rst");
    fs::write(&path, out).expect("failed to write config example RST");
    eprintln!("Generated {}", path.display());
}

/// Resolve a schema reference to get the actual schema object
fn resolve_ref<'a>(schema: &'a Schema, root: &'a RootSchema) -> Option<&'a Schema> {
    match schema {
        Schema::Object(obj) => {
            // Direct $ref
            if let Some(ref_path) = &obj.reference {
                let def_name = ref_path.strip_prefix("#/definitions/")?;
                return root.definitions.get(def_name);
            }

            // $ref wrapped in allOf (schemars does this for types with metadata)
            if let Some(subs) = &obj.subschemas {
                if let Some(all_of) = &subs.all_of {
                    for inner_schema in all_of {
                        if let Schema::Object(inner) = inner_schema {
                            if let Some(ref_path) = &inner.reference {
                                let def_name = ref_path.strip_prefix("#/definitions/")?;
                                return root.definitions.get(def_name);
                            }
                        }
                    }
                }
            }

            Some(schema)
        }
        Schema::Bool(_) => Some(schema),
    }
}

fn get_description(schema: &Schema, root: &RootSchema) -> String {
    // First try to resolve if it's a reference
    let resolved = resolve_ref(schema, root).unwrap_or(schema);

    match resolved {
        Schema::Object(obj) => obj
            .metadata
            .as_ref()
            .and_then(|m| m.description.clone())
            .unwrap_or_default(),
        Schema::Bool(_) => String::new(),
    }
}

fn get_nested_properties<'a>(
    schema: &'a Schema,
    root: &'a RootSchema,
) -> Option<&'a Map<String, Schema>> {
    // First resolve reference if needed
    let resolved = resolve_ref(schema, root)?;

    match resolved {
        Schema::Object(obj) => obj.object.as_ref().map(|o| &o.properties),
        Schema::Bool(_) => None,
    }
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
