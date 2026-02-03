use colored::Colorize;
use std::io::Write;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Mutex;

pub struct MullDiagnostics {
    debug_mode: AtomicBool,
    strict_mode: AtomicBool,
    quiet: AtomicBool,
    silent: AtomicBool,
    seen_progress: Mutex<bool>,
}

pub fn diagnostics_new() -> Box<MullDiagnostics> {
    Box::new(MullDiagnostics::new())
}

impl MullDiagnostics {
    pub fn new() -> Self {
        Self {
            debug_mode: AtomicBool::new(false),
            strict_mode: AtomicBool::new(false),
            quiet: AtomicBool::new(false),
            silent: AtomicBool::new(false),
            seen_progress: Mutex::new(false),
        }
    }

    fn prepare(&self) {
        let mut seen = self.seen_progress.lock().unwrap();
        if *seen {
            println!();
            let _ = std::io::stdout().flush();
            *seen = false;
        }
    }

    pub fn enable_debug_mode(&self) {
        self.debug_mode.store(true, Ordering::Relaxed);
    }

    pub fn enable_strict_mode(&self) {
        self.strict_mode.store(true, Ordering::Relaxed);
    }

    pub fn make_quiet(&self) {
        self.quiet.store(true, Ordering::Relaxed);
    }

    pub fn make_silent(&self) {
        self.silent.store(true, Ordering::Relaxed);
    }

    // C++ APIs

    pub fn info(&self, message: &str) {
        if self.quiet.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        println!("{} {}", "[info]".green(), message);
    }

    pub fn warning(&self, message: &str) {
        let strict = self.strict_mode.load(Ordering::Relaxed);
        if self.silent.load(Ordering::Relaxed) && !strict {
            return;
        }
        self.prepare();
        println!("{} {}", "[warning]".yellow(), message);
        if strict {
            println!(
                "{} {}",
                "[warning]".yellow(),
                "Strict Mode enabled: warning messages are treated as fatal errors. Exiting now."
            );
            std::process::exit(1);
        }
    }

    pub fn error(&self, message: &str) -> ! {
        self.prepare();
        println!("{} {}", "[error]".red(), message);
        println!(
            "{} {}",
            "[error]".red(),
            "Error messages are treated as fatal errors. Exiting now."
        );
        std::process::exit(1)
    }

    pub fn debug(&self, message: &str) {
        if !self.debug_mode.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        println!("{} {}", "[debug]".cyan(), message);
    }

    pub fn progress(&self, message: &str) {
        if self.quiet.load(Ordering::Relaxed) {
            return;
        }
        {
            let mut seen = self.seen_progress.lock().unwrap();
            *seen = true;
        }
        print!("{}", message);
        let _ = std::io::stdout().flush();
    }

    // Rust APIs

    pub fn info_fmt(&self, args: std::fmt::Arguments) {
        if self.quiet.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        println!("{} {}", "[info]".green(), args);
    }

    pub fn warning_fmt(&self, args: std::fmt::Arguments) {
        let strict = self.strict_mode.load(Ordering::Relaxed);
        if self.silent.load(Ordering::Relaxed) && !strict {
            return;
        }
        self.prepare();
        println!("{} {}", "[warning]".yellow(), args);
        if strict {
            println!(
                "{} {}",
                "[warning]".yellow(),
                "Strict Mode enabled: warning messages are treated as fatal errors. Exiting now."
            );
            std::process::exit(1);
        }
    }

    pub fn error_fmt(&self, args: std::fmt::Arguments) -> ! {
        self.prepare();
        println!("{} {}", "[error]".red(), args);
        println!(
            "{} {}",
            "[error]".red(),
            "Error messages are treated as fatal errors. Exiting now."
        );
        std::process::exit(1)
    }

    pub fn debug_fmt(&self, args: std::fmt::Arguments) {
        if !self.debug_mode.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        println!("{} {}", "[debug]".cyan(), args);
    }
}

impl Default for MullDiagnostics {
    fn default() -> Self {
        Self::new()
    }
}

// Usage: diag_info!(diag, "Found {} mutants", count);

#[macro_export]
macro_rules! diag_info {
    ($diag:expr, $($arg:tt)*) => {
        $diag.info_fmt(format_args!($($arg)*))
    };
}

#[macro_export]
macro_rules! diag_warning {
    ($diag:expr, $($arg:tt)*) => {
        $diag.warning_fmt(format_args!($($arg)*))
    };
}

#[macro_export]
macro_rules! diag_error {
    ($diag:expr, $($arg:tt)*) => {
        $diag.error_fmt(format_args!($($arg)*))
    };
}

#[macro_export]
macro_rules! diag_debug {
    ($diag:expr, $($arg:tt)*) => {
        $diag.debug_fmt(format_args!($($arg)*))
    };
}
