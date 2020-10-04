/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (%MULL_EXEC %s.exe 2>&1; test $? = 1) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines

NOTE: LLVM 9.0.0 suddenly prints 2 extra spaces and one extra "-" so adding a
NOTE: regex to accommodate.
CHECK:mull-cxx: for the{{ {1,3}-?}}-test-framework option: must be specified at least once!
**/

int main() {
  return 0;
}
