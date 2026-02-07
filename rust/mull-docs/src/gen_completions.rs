use clap::Command;
use clap_complete::generate;
use clap_complete::shells::{Bash, Fish, Zsh};
use std::fs::File;
use std::path::Path;

pub fn generate_all(mut cmd: Command, output_dir: &Path) {
    let name = cmd.get_name().to_string();

    let mut f = File::create(output_dir.join(format!("{name}.bash")))
        .expect("failed to create bash completion file");
    generate(Bash, &mut cmd, &name, &mut f);

    let mut f = File::create(output_dir.join(format!("{name}.zsh")))
        .expect("failed to create zsh completion file");
    generate(Zsh, &mut cmd, &name, &mut f);

    let mut f = File::create(output_dir.join(format!("{name}.fish")))
        .expect("failed to create fish completion file");
    generate(Fish, &mut cmd, &name, &mut f);

    eprintln!("Generated shell completions for {name}");
}
