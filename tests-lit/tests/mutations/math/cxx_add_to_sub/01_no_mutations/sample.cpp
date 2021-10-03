// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -compdb-path %S/compile_commands.json -compilation-flags="" -reporters=Elements -report-dir=%S -report-name=01_no_mutations %s.exe | %filecheck %s --dump-input=fail
CHECK:[info] No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
