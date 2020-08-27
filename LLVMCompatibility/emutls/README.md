# Emulated TLS helper

This TLS emulation layer imported from LLVM 10.0.1 must be linked if
the test subject contains any thread local variables. LLVM will use
emulated TLS when recompiling bitcode in the way that mull-cxx does.
The TLS emulation layer has changed very little between LLVM-3.9.x and
LLVM-10, so this library should work for older versions as well.

## Background

This is a workaround for the issue https://github.com/mull-project/mull/issues/743

Normally in an LLVM installation the function `__emutls_get_address`
will be found in a compiler-rt static library and would be linked in
by Clang if needed. This does not seem to be as straightforward when
recompiling LLVM bitcode on the fly like we do in Mull. Explicitly
loading the static library does not work because of problems with
`__emutls_get_address` to reach its external dependencies (pthread,
malloc).
As a workaround, building a separate dynamic library containing only
`emutls.c` and loading that library during dynamic linking in
`mull-cxx.cpp` avoids crashes when running tests on a subject with
thread local variables.

## Origin

All source files in this directory were copied unmodified from
https://github.com/llvm/llvm-project/blob/ef32c611aa214dea855364efd7ba451ec5ec3f74/compiler-rt/lib/builtins/
