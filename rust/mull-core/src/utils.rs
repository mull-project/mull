use std::time::Duration;

pub fn format_elapsed(elapsed: Duration) -> String {
    let total_ms = elapsed.as_millis();
    if total_ms >= 60_000 {
        let mins = total_ms / 60_000;
        let secs = (total_ms % 60_000) as f64 / 1000.0;
        format!("{}m{:.1}s", mins, secs)
    } else if total_ms >= 1_000 {
        let secs = total_ms as f64 / 1000.0;
        format!("{:.2}s", secs)
    } else {
        format!("{}ms", total_ms)
    }
}
