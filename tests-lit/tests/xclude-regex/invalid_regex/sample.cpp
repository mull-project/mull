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
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/sample.cpp -o Output/sample.cpp-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[warning] Invalid regex for exclude-path:{{.*}}
CHECK:[warning] Invalid regex for include-path:{{.*}}
*/
