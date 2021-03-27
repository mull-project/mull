int doSomething(int arg1, int arg2) {
  return arg1 + arg2;
}

int main() {
  if (doSomething(2, 3) != 5) {
    return 1;
  }
  return 0;
}

// clang-format off

// RUN: cd / && %clang_cc %s -fembed-bitcode -g -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -mutators=cxx_add_to_sub -output=%s.mutated-keep.exe -keep-executable %s.exe
// RUN: test -f %s.mutated-keep.exe
// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -mutators=cxx_add_to_sub -output=%s.mutated-delete.exe %s.exe
// RUN: test ! -f %s.mutated-delete.exe