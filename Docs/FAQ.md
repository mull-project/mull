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

## Which test frameworks does Mull support?

Currently Mull supports 3 test frameworks: GoogleTest, CustomTest, SimpleTest.

Also there is experimental support of Rust's test framework and XCTest (Swift, Linux only).

### Google Test

Projects we analyzed with GoogleTest:

- LLVM
- [fmt](https://github.com/fmtlib/fmt).
- Mull is also tested using GoogleTest. The scripts found in [mull-auto-test](https://github.com/mull-project/mull-auto-test) allow Mull to test its own code
and tests.

### Custom Test

This framework is designed to handle projects with exotic test suites. Every
test suite is different so the framework might be updated often.

Projects we analyzed with CustomTest:

- OpenSSL
- [openlibm](https://github.com/JuliaLang/openlibm)
- [newlib/libm](https://sourceware.org/newlib/)

### Simple Test

This test framework was used to bootstrap Mull: test is a function that has
`test*` name, for example `test_count`.

So far we didn't encounter any real-world test suite that would use this
convention so `SimpleTest` is only used for Mull's own tests.
