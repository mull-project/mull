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
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -fembed-bitcode -g -O0 Output/sandbox/main.cpp -o Output/main.cpp.exe

RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_cxx -mutators=cxx_eq_to_ne -linker-flags="%sysroot" --linker=%clang_cxx -debug main.cpp.exe --report-name test --reporters Patches --reporters IDE; test $? = 0; ls -R %S/Output/test-patches; cat %S/Output/test-patches/killed-Output_sandbox_main_cpp-cxx_eq_to_ne-L2-C12.patch) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[debug] Writing Patchfile: {{.*}}
CHECK:[info] Patchfiles can be found at './test-patches'
CHECK:{{.*}}main_cpp{{.*}}
CHECK:--- a{{.*}}/Output/sandbox/main.cpp 0
CHECK:+{{\s+}}return a != b;
CHECK:--
CHECK:Build Date: {{.*}}
CHECK:LLVM Version: {{.*}}
CHECK:Mull Version: {{.*}}

*/
