use std::collections::HashSet;

/// Expand mutator group names into individual mutator IDs.
///
/// Maps groups like "cxx_all" or "cxx_arithmetic" to their constituent mutator IDs.
/// Unknown group names are passed through as-is (assumed to be individual mutator IDs).
pub fn expand_mutator_groups(groups: &[&str]) -> HashSet<String> {
    let mut result = HashSet::new();
    for group in groups {
        expand_group(group, &mut result);
    }
    result
}

/// Returns all group definitions as (group_name, children_description) pairs.
/// Used for generating command-line help text.
/// Groups are returned in sorted order by name.
pub fn get_group_definitions() -> Vec<(String, String)> {
    let mut groups = vec![
        ("cxx_all", "cxx_assignment, cxx_increment, cxx_decrement, cxx_arithmetic, cxx_comparison, cxx_boundary, cxx_bitwise, cxx_calls"),
        ("cxx_default", "cxx_increment, cxx_arithmetic, cxx_comparison, cxx_boundary"),
        ("cxx_assignment", "cxx_bitwise_assignment, cxx_arithmetic_assignment, cxx_const_assignment"),
        ("cxx_arithmetic_assignment", "cxx_add_assign_to_sub_assign, cxx_sub_assign_to_add_assign, cxx_mul_assign_to_div_assign, cxx_div_assign_to_mul_assign, cxx_rem_assign_to_div_assign"),
        ("cxx_bitwise_assignment", "cxx_and_assign_to_or_assign, cxx_or_assign_to_and_assign, cxx_xor_assign_to_or_assign, cxx_lshift_assign_to_rshift_assign, cxx_rshift_assign_to_lshift_assign"),
        ("cxx_const_assignment", "cxx_assign_const, cxx_init_const"),
        ("cxx_increment", "cxx_pre_inc_to_pre_dec, cxx_post_inc_to_post_dec"),
        ("cxx_decrement", "cxx_pre_dec_to_pre_inc, cxx_post_dec_to_post_inc"),
        ("cxx_arithmetic", "cxx_minus_to_noop, cxx_add_to_sub, cxx_sub_to_add, cxx_mul_to_div, cxx_div_to_mul, cxx_rem_to_div"),
        ("cxx_bitwise", "cxx_bitwise_not_to_noop, cxx_and_to_or, cxx_or_to_and, cxx_xor_to_or, cxx_lshift_to_rshift, cxx_rshift_to_lshift"),
        ("cxx_logical", "cxx_remove_negation"),
        ("cxx_comparison", "cxx_eq_to_ne, cxx_ne_to_eq, cxx_le_to_gt, cxx_lt_to_ge, cxx_ge_to_lt, cxx_gt_to_le"),
        ("cxx_boundary", "cxx_le_to_lt, cxx_lt_to_le, cxx_ge_to_gt, cxx_gt_to_ge"),
        ("cxx_calls", "cxx_remove_void_call, cxx_replace_scalar_call"),
        ("experimental", "negate_mutator, cxx_logical"),
    ];

    groups.sort_by(|a, b| a.0.cmp(b.0));
    groups
        .iter()
        .map(|(name, desc)| (name.to_string(), desc.to_string()))
        .collect()
}

fn expand_group(group: &str, result: &mut HashSet<String>) {
    match group {
        // Meta groups
        "cxx_all" => {
            expand_group("cxx_assignment", result);
            expand_group("cxx_increment", result);
            expand_group("cxx_decrement", result);
            expand_group("cxx_arithmetic", result);
            expand_group("cxx_comparison", result);
            expand_group("cxx_boundary", result);
            expand_group("cxx_bitwise", result);
            expand_group("cxx_calls", result);
        }
        "cxx_default" => {
            expand_group("cxx_increment", result);
            expand_group("cxx_arithmetic", result);
            expand_group("cxx_comparison", result);
            expand_group("cxx_boundary", result);
        }
        "cxx_assignment" => {
            expand_group("cxx_bitwise_assignment", result);
            expand_group("cxx_arithmetic_assignment", result);
            expand_group("cxx_const_assignment", result);
        }
        "experimental" => {
            result.insert("negate_mutator".to_string());
            expand_group("cxx_logical", result);
        }

        // Leaf groups
        "cxx_calls" => {
            result.insert("cxx_remove_void_call".to_string());
            result.insert("cxx_replace_scalar_call".to_string());
        }
        "cxx_const_assignment" => {
            result.insert("cxx_assign_const".to_string());
            result.insert("cxx_init_const".to_string());
        }
        "cxx_bitwise_assignment" => {
            result.insert("cxx_and_assign_to_or_assign".to_string());
            result.insert("cxx_or_assign_to_and_assign".to_string());
            result.insert("cxx_xor_assign_to_or_assign".to_string());
            result.insert("cxx_lshift_assign_to_rshift_assign".to_string());
            result.insert("cxx_rshift_assign_to_lshift_assign".to_string());
        }
        "cxx_arithmetic_assignment" => {
            result.insert("cxx_add_assign_to_sub_assign".to_string());
            result.insert("cxx_sub_assign_to_add_assign".to_string());
            result.insert("cxx_mul_assign_to_div_assign".to_string());
            result.insert("cxx_div_assign_to_mul_assign".to_string());
            result.insert("cxx_rem_assign_to_div_assign".to_string());
        }
        "cxx_increment" => {
            result.insert("cxx_pre_inc_to_pre_dec".to_string());
            result.insert("cxx_post_inc_to_post_dec".to_string());
        }
        "cxx_decrement" => {
            result.insert("cxx_pre_dec_to_pre_inc".to_string());
            result.insert("cxx_post_dec_to_post_inc".to_string());
        }
        "cxx_arithmetic" => {
            result.insert("cxx_minus_to_noop".to_string());
            result.insert("cxx_add_to_sub".to_string());
            result.insert("cxx_sub_to_add".to_string());
            result.insert("cxx_mul_to_div".to_string());
            result.insert("cxx_div_to_mul".to_string());
            result.insert("cxx_rem_to_div".to_string());
        }
        "cxx_bitwise" => {
            result.insert("cxx_bitwise_not_to_noop".to_string());
            result.insert("cxx_and_to_or".to_string());
            result.insert("cxx_or_to_and".to_string());
            result.insert("cxx_xor_to_or".to_string());
            result.insert("cxx_lshift_to_rshift".to_string());
            result.insert("cxx_rshift_to_lshift".to_string());
        }
        "cxx_logical" => {
            result.insert("cxx_remove_negation".to_string());
        }
        "cxx_comparison" => {
            result.insert("cxx_eq_to_ne".to_string());
            result.insert("cxx_ne_to_eq".to_string());
            result.insert("cxx_le_to_gt".to_string());
            result.insert("cxx_lt_to_ge".to_string());
            result.insert("cxx_ge_to_lt".to_string());
            result.insert("cxx_gt_to_le".to_string());
        }
        "cxx_boundary" => {
            result.insert("cxx_le_to_lt".to_string());
            result.insert("cxx_lt_to_le".to_string());
            result.insert("cxx_ge_to_gt".to_string());
            result.insert("cxx_gt_to_ge".to_string());
        }

        // Not a group, treat as individual mutator ID
        _ => {
            result.insert(group.to_string());
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_expand_single_mutator() {
        let result = expand_mutator_groups(&["cxx_add_to_sub"]);
        assert_eq!(result.len(), 1);
        assert!(result.contains("cxx_add_to_sub"));
    }

    #[test]
    fn test_expand_arithmetic_group() {
        let result = expand_mutator_groups(&["cxx_arithmetic"]);
        assert!(result.contains("cxx_add_to_sub"));
        assert!(result.contains("cxx_sub_to_add"));
        assert!(result.contains("cxx_mul_to_div"));
        assert!(result.contains("cxx_div_to_mul"));
        assert!(result.contains("cxx_rem_to_div"));
        assert!(result.contains("cxx_minus_to_noop"));
        assert_eq!(result.len(), 6);
    }

    #[test]
    fn test_expand_cxx_all() {
        let result = expand_mutator_groups(&["cxx_all"]);
        // Should contain mutators from all groups
        assert!(result.contains("cxx_add_to_sub")); // arithmetic
        assert!(result.contains("cxx_eq_to_ne")); // comparison
        assert!(result.contains("cxx_le_to_lt")); // boundary
        assert!(result.contains("cxx_and_to_or")); // bitwise
        assert!(result.contains("cxx_pre_inc_to_pre_dec")); // increment
        assert!(result.contains("cxx_pre_dec_to_pre_inc")); // decrement
        assert!(result.contains("cxx_remove_void_call")); // calls
        assert!(result.contains("cxx_assign_const")); // const_assignment
    }

    #[test]
    fn test_expand_multiple_groups() {
        let result = expand_mutator_groups(&["cxx_arithmetic", "cxx_comparison"]);
        assert!(result.contains("cxx_add_to_sub"));
        assert!(result.contains("cxx_eq_to_ne"));
    }
}
