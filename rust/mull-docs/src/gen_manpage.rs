use clap::Command;
use std::fs;
use std::path::Path;

pub fn generate(cmd: Command, filename: &str, output_dir: &Path) {
    let man = clap_mangen::Man::new(cmd);
    let mut buf = Vec::new();
    man.render(&mut buf).expect("failed to render man page");
    let path = output_dir.join(filename);
    fs::write(&path, buf).expect("failed to write man page");
    eprintln!("Generated {}", path.display());
}
