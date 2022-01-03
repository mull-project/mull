int equal(int a, int b) {
  return a == b;
}

int main() {
  return equal(2, 2) != 1;
}

// clang-format off
/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/sample.cpp %S/Output/sandbox/sample.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -fembed-bitcode -g -O0 Output/sandbox/sample.cpp -o Output/sample.cpp.exe
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/sample.cpp -o Output/sample.cpp-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_cxx -mutators=cxx_eq_to_ne -linker-flags="%sysroot" --linker=%clang_cxx -debug sample.cpp.exe --reporters IDE --include-path=.*cpp --exclude-path=Output/.*; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK-NOT:[warning] Invalid regex{{.*}}
*/
