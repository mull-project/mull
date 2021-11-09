void foobar(int b, bool x) {
  b += x?4:5;
}

int main() {
// clang-format off
  foobar (4,
      false);
  foobar (4 +
      7 +
      9 +
      10,
      false);
// clang-format on
  return 0;
}

// clang-format off
/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -fembed-bitcode -g -O0 Output/sandbox/main.cpp -o Output/main.cpp.exe

RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_cxx -mutators=cxx_remove_void_call -linker-flags="%sysroot" --linker=%clang_cxx -debug main.cpp.exe --report-name test --reporters Patches --reporters IDE; test $? = 0; ls -R %S/Output/test-patches; cat %S/Output/test-patches/survived-Output_sandbox_main_cpp-cxx_remove_void_call-L7-C3.patch; cat %S/Output/test-patches/survived-Output_sandbox_main_cpp-cxx_remove_void_call-L9-C3.patch) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[debug] Writing Patchfile: {{.*}}
CHECK:[info] Patchfiles can be found at './test-patches'
CHECK:{{.*}}main_cpp-cxx_remove_void{{.*}}
CHECK:--- a{{.*}}/Output/sandbox/main.cpp 0
CHECK:-{{\s+}}false);
CHECK:-{{\s+}}7 +
CHECK:-{{\s+}}9 +
CHECK:-{{\s+}}false);
CHECK:+{{\s+}};

*/
