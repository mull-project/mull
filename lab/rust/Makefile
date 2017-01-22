
LLI ?= /usr/local/opt/llvm/bin/lli

default: compile run

compile:
	mkdir -p target/debug
	rustc example.rs -Z print-link-args --verbose --crate-name example --crate-type lib -g --test --out-dir target/debug --emit=dep-info,link,llvm-ir,llvm-bc -L dependency=target/debug -L dependency=target/debug/deps

run:
	./target/debug/example

run_normal:
	cargo test

lli: 
	$(LLI) -load \
           /Users/Stanislaw/.rustup/toolchains/stable-x86_64-apple-darwin/lib/libstd-f5a209a9.dylib \
           -load \
           /Users/Stanislaw/.rustup/toolchains/stable-x86_64-apple-darwin/lib/libtest-f5a209a9.dylib \
           ./target/debug/example.bc

clean:
	rm -rfv target/

