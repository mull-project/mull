fn main() {
    let source = std::fs::read_to_string("src/lib.rs").expect("Failed to read src/lib.rs");
    let tokens: proc_macro2::TokenStream = source.parse().expect("Failed to parse Rust source");
    let opt = cxx_gen::Opt::default();
    let generated =
        cxx_gen::generate_header_and_cc(tokens, &opt).expect("Failed to generate cxx bridge");

    let out_dir = std::env::var("OUT_DIR").unwrap();
    std::fs::write(format!("{out_dir}/core.rs.h"), &generated.header).unwrap();
    std::fs::write(format!("{out_dir}/core.rs.cc"), &generated.implementation).unwrap();

    println!("cargo:rerun-if-changed=src/lib.rs");
}
