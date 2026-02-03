use clap::CommandFactory;
use mull_config::MullConfigSpec;
use std::path::Path;

mod gen_example_yaml;
mod gen_manpage;
mod gen_rst;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 2 {
        eprintln!("Usage: mull-docs <output-directory>");
        std::process::exit(1);
    }

    let output_dir = Path::new(&args[1]);
    std::fs::create_dir_all(output_dir).expect("failed to create output directory");

    let cmd = MullConfigSpec::command();

    gen_manpage::generate(cmd, output_dir);
    gen_rst::generate(output_dir);
    gen_example_yaml::generate(output_dir);

    eprintln!("All docs generated in {}", output_dir.display());
}
