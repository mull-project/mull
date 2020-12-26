#ifndef FLAG
#error "FLAG is not defined"
#endif

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
REQUIRES: LLVM_8_OR_HIGHER
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -DFLAG=1 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -disable-junk-detection -mutators=cxx_add_to_sub -mutators=remove_void_function_mutator -reporters=IDE -ide-reporter-show-killed %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-RECORD-COMMAND-LINE
WITHOUT-RECORD-COMMAND-LINE-NOT:Found compilation flags in the input bitcode
WITHOUT-RECORD-COMMAND-LINE:{{^.*}}sample.cpp:5:13: warning: Survived: Remove Void Call: removed llvm.dbg.declare [remove_void_function_mutator]{{$}}

RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -DFLAG=1 -grecord-command-line %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -mutators=remove_void_function_mutator -reporters=IDE -ide-reporter-show-killed %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-RECORD-COMMAND-LINE
WITH-RECORD-COMMAND-LINE:[info] Found compilation flags in the input bitcode
WITH-RECORD-COMMAND-LINE-NOT:{{^.*[Ee]rror.*$}}
WITH-RECORD-COMMAND-LINE:[info] Killed mutants (1/1):
**/
