use colored::{Color, Colorize};
use std::io::Write;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Mutex;

pub struct MullDiagnostics {
    debug_mode: AtomicBool,
    strict_mode: AtomicBool,
    quiet: AtomicBool,
    silent: AtomicBool,
    use_colors: bool,
    seen_progress: Mutex<bool>,
    writer: Mutex<Box<dyn Write>>,
}

impl MullDiagnostics {
    pub fn new() -> Self {
        Self::with_writer(Box::new(std::io::stdout()), true)
    }

    pub fn with_writer(writer: Box<dyn Write>, use_colors: bool) -> Self {
        Self {
            debug_mode: AtomicBool::new(false),
            strict_mode: AtomicBool::new(false),
            quiet: AtomicBool::new(false),
            silent: AtomicBool::new(false),
            use_colors,
            seen_progress: Mutex::new(false),
            writer: Mutex::new(writer),
        }
    }

    fn prefix(&self, label: &str, color: Color) -> String {
        if self.use_colors {
            label.color(color).to_string()
        } else {
            label.to_string()
        }
    }

    fn prepare(&self) {
        let mut seen = self.seen_progress.lock().unwrap();
        if *seen {
            let mut w = self.writer.lock().unwrap();
            let _ = writeln!(w);
            let _ = w.flush();
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

    // C++ FFI APIs

    pub fn info(&self, message: &str) {
        self.info_fmt(format_args!("{}", message));
    }

    pub fn warning(&self, message: &str) {
        self.warning_fmt(format_args!("{}", message));
    }

    pub fn error(&self, message: &str) -> ! {
        self.error_fmt(format_args!("{}", message));
    }

    pub fn debug(&self, message: &str) {
        self.debug_fmt(format_args!("{}", message));
    }

    pub fn progress(&self, message: &str) {
        if self.quiet.load(Ordering::Relaxed) {
            return;
        }
        {
            let mut seen = self.seen_progress.lock().unwrap();
            *seen = true;
        }
        let mut w = self.writer.lock().unwrap();
        let _ = write!(w, "{}", message);
        let _ = w.flush();
    }

    pub fn raw_fmt(&self, args: std::fmt::Arguments) {
        if self.quiet.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        let mut w = self.writer.lock().unwrap();
        let _ = writeln!(w, "{}", args);
    }

    pub fn info_fmt(&self, args: std::fmt::Arguments) {
        if self.quiet.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        let mut w = self.writer.lock().unwrap();
        let _ = writeln!(w, "{} {}", self.prefix("[info]", Color::Green), args);
    }

    pub fn warning_fmt(&self, args: std::fmt::Arguments) {
        let strict = self.strict_mode.load(Ordering::Relaxed);
        if self.silent.load(Ordering::Relaxed) && !strict {
            return;
        }
        self.prepare();
        let prefix = self.prefix("[warning]", Color::Yellow);
        let mut w = self.writer.lock().unwrap();
        let _ = writeln!(w, "{} {}", prefix, args);
        if strict {
            let _ = writeln!(
                w,
                "{} {}",
                prefix,
                "Strict Mode enabled: warning messages are treated as fatal errors. Exiting now."
            );
            std::process::exit(1);
        }
    }

    pub fn error_fmt(&self, args: std::fmt::Arguments) -> ! {
        self.prepare();
        let prefix = self.prefix("[error]", Color::Red);
        let mut w = self.writer.lock().unwrap();
        let _ = writeln!(w, "{} {}", prefix, args);
        let _ = writeln!(
            w,
            "{} {}",
            prefix, "Error messages are treated as fatal errors. Exiting now."
        );
        std::process::exit(1)
    }

    pub fn debug_fmt(&self, args: std::fmt::Arguments) {
        if !self.debug_mode.load(Ordering::Relaxed) {
            return;
        }
        self.prepare();
        let mut w = self.writer.lock().unwrap();
        let _ = writeln!(w, "{} {}", self.prefix("[debug]", Color::Cyan), args);
    }
}

impl Default for MullDiagnostics {
    fn default() -> Self {
        Self::new()
    }
}

// Usage: diag_info!(diag, "Found {} mutants", count);

#[macro_export]
macro_rules! diag_raw {
    ($diag:expr, $($arg:tt)*) => {
        $diag.raw_fmt(format_args!($($arg)*))
    };
}

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
