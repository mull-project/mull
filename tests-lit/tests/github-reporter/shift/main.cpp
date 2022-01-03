int bitwise_left_shift(int a, int b) {
  return a << b;
}

int main() {
  auto j = bitwise_left_shift(1, 2);
  return 0;
}

// clang-format off
/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -fembed-bitcode -g -O0 Output/sandbox/main.cpp -o Output/main.cpp-ebc.exe
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/main.cpp -o Output/main.cpp-ir.exe

RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_cxx -mutators=cxx_bitwise -linker-flags="%sysroot" --linker=%clang_cxx -debug main.cpp-ebc.exe --report-name test --reporters GithubAnnotations --reporters IDE; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_runner main.cpp-ir.exe --report-name test --reporters GithubAnnotations --reporters IDE; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Github Annotations:
CHECK:{{::}}warning file={{.*}}/main.cpp,line=2,col=12,endLine=2,endColumn=14::[cxx_lshift_to_rshift] Replaced << with >>
*/
