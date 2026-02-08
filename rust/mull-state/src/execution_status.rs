use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
#[repr(u8)]
pub enum ExecutionStatus {
    #[default]
    Invalid = 0,
    Failed = 1,
    Passed = 2,
    Timedout = 3,
    Crashed = 4,
    AbnormalExit = 5,
    DryRun = 6,
    FailFast = 7,
    NotCovered = 8,
}

impl ExecutionStatus {
    pub fn from_i32(value: i32) -> Self {
        match value {
            0 => ExecutionStatus::Invalid,
            1 => ExecutionStatus::Failed,
            2 => ExecutionStatus::Passed,
            3 => ExecutionStatus::Timedout,
            4 => ExecutionStatus::Crashed,
            5 => ExecutionStatus::AbnormalExit,
            6 => ExecutionStatus::DryRun,
            7 => ExecutionStatus::FailFast,
            8 => ExecutionStatus::NotCovered,
            _ => ExecutionStatus::Invalid,
        }
    }
}

impl fmt::Display for ExecutionStatus {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let s = match self {
            ExecutionStatus::Invalid => "Invalid",
            ExecutionStatus::Failed => "Failed",
            ExecutionStatus::Passed => "Passed",
            ExecutionStatus::Timedout => "Timedout",
            ExecutionStatus::Crashed => "Crashed",
            ExecutionStatus::AbnormalExit => "AbnormalExit",
            ExecutionStatus::DryRun => "DryRun",
            ExecutionStatus::FailFast => "FailFast",
            ExecutionStatus::NotCovered => "NotCovered",
        };
        write!(f, "{}", s)
    }
}
