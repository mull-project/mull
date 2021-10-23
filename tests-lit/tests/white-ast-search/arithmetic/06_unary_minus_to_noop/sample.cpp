int unary_minus(int x) {
  return -x;
}

int main() {
  return ! (unary_minus(1) == -1);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -debug -mutators=cxx_minus_to_noop -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Unary Minus to Noop": {{.*}}sample.cpp:2:10 (end: 2:12)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:10: warning: Killed: Replaced -x with x [cxx_minus_to_noop]{{$}}
CHECK:  return -x;
CHECK:         ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
