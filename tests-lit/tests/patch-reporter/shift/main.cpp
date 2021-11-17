int bitwise_left_shift(int a, int b) {
  return a << b;
}

int main() {
  return ! (bitwise_left_shift(1, 2) == 4);
}

// clang-format off
/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -fembed-bitcode -g -O0 Output/sandbox/main.cpp -o Output/main.cpp.exe

RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_cxx -mutators=cxx_bitwise -linker-flags="%sysroot" --linker=%clang_cxx -debug main.cpp.exe --report-name test --reporters Patches --reporters IDE; test $? = 0; ls -R %S/Output/test-patches; cat %S/Output/test-patches/killed-Output_sandbox_main_cpp-cxx_lshift_to_rshift-L2-C12.patch) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[debug] Writing Patchfile: {{.*}}
CHECK:[info] Patchfiles can be found at './test-patches'
CHECK:killed-{{.*}}main_cpp{{.*}}
CHECK:--- a/{{.*}}/Output/sandbox/main.cpp 0
CHECK:+++ b/{{.*}}/Output/sandbox/main.cpp 0
CHECK:+{{\s+}}return a >> b;

*/
