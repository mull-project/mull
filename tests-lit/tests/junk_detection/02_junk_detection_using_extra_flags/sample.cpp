#ifndef FLAG
#error "FLAG is not defined"
#endif

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 -DFLAG=1 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: %MULL_EXEC -test-framework CustomTest -mutators=all -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION
RUN: %MULL_EXEC -test-framework CustomTest -mutators=all -reporters=IDE -ide-reporter-show-killed -compilation-flags '-DWRONG_FLAG=1' %s.exe 2>&1 | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-NO-FLAG
RUN: %MULL_EXEC -test-framework CustomTest -mutators=all -reporters=IDE -ide-reporter-show-killed -compilation-flags '-DFLAG=1' %s.exe 2>&1 | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAG

WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:5:13: warning: Survived: Remove Void Call: removed llvm.dbg.declare [remove_void_function_mutator]{{$}}

WITH-JUNK-DETECTION-NO-FLAG:{{^.*}}sample.cpp:2:2: error: "FLAG is not defined"
WITH-JUNK-DETECTION-NO-FLAG:#error "FLAG is not defined"
WITH-JUNK-DETECTION-NO-FLAG: ^
WITH-JUNK-DETECTION-NO-FLAG:[warning] Cannot parse file: '{{.*}}sample.cpp':
WITH-JUNK-DETECTION-NO-FLAG:mull-cxx {{.*}}sample.cpp{{$}}
WITH-JUNK-DETECTION-NO-FLAG:Make sure that the flags provided to Mull are the same flags that are used for normal compilation.
TODO: It is interesting why there is no junk even if we have the error above.
WITH-JUNK-DETECTION-NO-FLAG:[info] Killed mutants (1/1):

WITH-JUNK-DETECTION-WITH-FLAG-NOT:#error "FLAG is not defined"
WITH-JUNK-DETECTION-WITH-FLAG-NOT:{{^.*[Ee]rror.*$}}
WITH-JUNK-DETECTION-WITH-FLAG:[info] Killed mutants (1/1):
**/
