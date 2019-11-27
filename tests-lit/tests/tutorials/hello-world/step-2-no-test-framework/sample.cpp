/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: (%MULL_EXEC %s.exe 2>&1; test $? = 1) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines
; CHECK:mull-cxx: for the -test-framework option: must be specified at least once!
**/

int main() {
  return 0;
}
