use crate::ExecutionResult;
use crate::Mutant;
use std::collections::HashMap;

#[derive(Debug, Default)]
pub struct ExecutionState {
    pub info: HashMap<String, String>,
    pub execution_results: HashMap<Mutant, ExecutionResult>,
}
