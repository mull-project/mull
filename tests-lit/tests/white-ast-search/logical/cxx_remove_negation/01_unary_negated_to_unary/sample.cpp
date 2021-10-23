bool isFalse(bool a) {
  return !a;
}

int main() {
  return (isFalse(true) == false) == false;
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -debug -mutators=cxx_remove_negation -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}

CHECK:[info] Applying filter: junk (threads: 1)

TODO: IDE reporter reports location "!a" but we would rather want to see the location of '!'.
CHECK:[debug] CXXJunkDetector: mutation "Remove Unary Negation": {{.*}}sample.cpp:2:10 (end: 2:12)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:10: warning: Killed: Replaced !a with a [cxx_remove_negation]{{$}}
CHECK:  return !a;
CHECK:         ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
