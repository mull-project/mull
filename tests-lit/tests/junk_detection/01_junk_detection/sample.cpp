int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
RUN: sed -e "s:%PWD:%s:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR
RUN: %MULL_EXEC -linker=%clang_cxx -workers=1 -disable-junk-detection -mutators=cxx_add_to_sub -mutators=remove_void_function_mutator -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION
RUN: %MULL_EXEC -linker=%clang_cxx -workers=1 -mutators=cxx_add_to_sub -mutators=remove_void_function_mutator -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION

WITHOUT-JUNK-DETECTION:{{^.*}}[info] Running mutants (threads: 1){{$}}
WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:1:13: warning: Survived: Remove Void Call: removed llvm.dbg.declare [remove_void_function_mutator]{{$}}
WITHOUT-JUNK-DETECTION-NOT:[info] Mutation score: 100%

WITH-JUNK-DETECTION:{{^.*}}[info] Running mutants (threads: 1){{$}}
WITH-JUNK-DETECTION:[info] Killed mutants (1/1):
WITH-JUNK-DETECTION:{{^.*}}/sample.cpp:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
WITH-JUNK-DETECTION:  return a + b;
WITH-JUNK-DETECTION:           ^
WITH-JUNK-DETECTION:[info] All mutations have been killed
WITH-JUNK-DETECTION:[info] Mutation score: 100%
**/
