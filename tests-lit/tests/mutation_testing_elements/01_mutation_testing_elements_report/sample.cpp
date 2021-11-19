void void_sum(int a, int b, int *result) {
  *result = a + b;
}
int sum(int a, int b) {
  int result = 0;
  void_sum(a, b, &result);
  return result;
}

int main() {
  return ! (sum(2, 3) == 5);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -debug -mutators=cxx_remove_void_call -reporters=IDE -reporters=Elements --report-dir=%S/Output --report-name=sample -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: [[ -f %S/Output/sample.json ]]
RUN: cat %S/Output/sample.json | filecheck %s --check-prefix=CHECK-JSON
CHECK-JSON: "config": {"Build Date": "{{.*}}", "CommandLine": "{{.*}}", {{.*}}, "URL": "https://github.com/mull-project/mull"}{{.*}} "mutants": [{"id": "cxx_remove_void_call", "location": {"end": {"column": 26, "line": 6}, "start": {"column": 3, "line": 6}}, "mutatorName": "Removed the call to the function", "replacement": "", "status": "Killed"}]{{.*}} "framework": {"brandingInformation": {"homepageUrl": "https://github.com/mull-project/mull"}, "name": "Mull", "version": "{{.*}}, LLVM {{.*}}"}

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:6:3: warning: Killed: Removed the call to the function [cxx_remove_void_call]
CHECK:  void_sum(a, b, &result);
CHECK:  ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
