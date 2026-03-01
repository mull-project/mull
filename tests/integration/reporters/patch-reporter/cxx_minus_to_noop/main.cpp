int main() {
  int x = -1;
  int y = -x;

  return y != 1; // ensuring zero exit
}

// clang-format off
/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/main.cpp -o Output/main.cpp-ir.exe

RUN: cd %S/Output; (unset TERM; %mull_runner -debug ./main.cpp-ir.exe --report-name test-ir --reporters Patches --reporters IDE --reporters SQLite; test $? = 0; ls -R %S/Output/test-ir-patches; cd %S/Output/test-ir-patches; cat `ls`) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: cd %S/Output; (unset TERM; %mull_reporter -debug test-ir.sqlite --report-name test-ir2 --reporters Patches; test $? = 0; ls -R %S/Output/test-ir2-patches; cd %S/Output/test-ir2-patches; cat `ls`) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines


CHECK:[debug] Writing Patchfile: {{.*}}
CHECK:[info] Patchfiles can be found at './test{{.*}}-patches'
CHECK:killed-{{.*}}
CHECK:--- a/{{.*}}/Output/sandbox/main.cpp 0
CHECK:+++ b/{{.*}}/Output/sandbox/main.cpp 0
CHECK:+{{\s+}}int y = x;

*/
