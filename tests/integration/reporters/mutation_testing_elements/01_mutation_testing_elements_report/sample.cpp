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
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %S && %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-ir.exe
RUN: unset TERM; %mull_runner -debug -reporters=IDE -reporters=Elements --report-dir=%S/Output --report-name=sample-ir -ide-reporter-show-killed %s-ir.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: [[ -f %S/Output/sample-ir.json ]]
RUN: cat %S/Output/sample-ir.json | filecheck %s --check-prefix=CHECK-JSON
CHECK-JSON: "config": {"Build Date": "{{.*}}", {{.*}} "URL": "https://github.com/mull-project/mull"}{{.*}} "mutants": [{"id": "cxx_remove_void_call", "location": {"end": {"column": 26, "line": 6}, "start": {"column": 3, "line": 6}}, "mutatorName": "Removed the call to the function", "replacement": "", "status": "Killed"}]{{.*}} "framework": {"brandingInformation": {"homepageUrl": "https://github.com/mull-project/mull"}, "name": "Mull", "version": "{{.*}}, LLVM {{.*}}"}

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:6:3: warning: Killed: Removed the call to the function [cxx_remove_void_call]
CHECK:  void_sum(a, b, &result);
CHECK:  ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
