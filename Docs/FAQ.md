# Mull FAQ

## Which programming languages does Mull support?

Theoretically Mull should work with any programming language that compiles to
LLVM IR. Practially, however, we have been mostly focused on a good C/C++
support.

We tried Rust and it worked well. Rudimentary support of Rust is in the tree:
see [lib/Rust](https://github.com/mull-project/mull/tree/master/lib/Rust).

Mull is quite close to support Objective-C and Swift. One specific blocker
for these languages is a lack of ObjC support by the linker/loader that are used
in LLVM JIT. See this issue for details: [Initial support of Objective-C](https://github.com/mull-project/mull/issues/32).

