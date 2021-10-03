bool foo(bool a, bool b) {
  if (a && b) {
    return true;
  } else {
    return false;
  }
}

int main() {
  return ! (foo(false, true) == false);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -debug -mutators=cxx_logical_and_to_or -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Logical And to Or": {{.*}}sample.cpp:2:9 (end: 2:11)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:9: warning: Killed: Replaced && with || [cxx_logical_and_to_or]{{$}}
CHECK:  if (a && b) {
CHECK:        ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
