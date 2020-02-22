/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -compdb-path %S/compile_commands.json -compilation-flags="" -reporters=Elements -report-dir=%S -report-name=01_no_mutations %s.exe | %FILECHECK_EXEC %s
; CHECK:[info] No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
