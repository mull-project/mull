void void_sum(int a, int b, int *result) {
  *result = a + b;
}
int sum(int a, int b) {
  int result = 0;
  void_sum(a, b, &result);
  return result;
}

int main() {
  return !(sum(2, 3) == 5);
}

// clang-format off

/**
RUN: sed -e "s:%PWD:%PWD:g" compile_commands.json.template > compile_commands.json
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-ir.exe
RUN: %mull_runner -debug -reporters=IDE -reporters=Elements --reporters=SQLite --report-dir=Output --report-name=sample-ir -ide-reporter-show-killed %s-ir.exe
RUN: %mull_reporter -debug -reporters=IDE -reporters=Elements --report-dir=Output --report-name=sample-ir2 -ide-reporter-show-killed Output/sample-ir.sqlite
RUN: [[ -f Output/sample-ir.txt ]]
RUN: cat Output/sample-ir.txt | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: [[ -f Output/sample-ir.json ]]
RUN: [[ -f Output/sample-ir2.json ]]
TODO: Use jq/structured checks
RU: cat Output/sample-ir.json | filecheck %s --check-prefix=CHECK-JSON
RU: cat Output/sample-ir2.json | filecheck %s --check-prefix=CHECK-JSON
CHECK-JSON: "config": {{{.*}} "URL": "https://github.com/mull-project/mull"}{{.*}} "mutants": [{"id": "cxx_remove_void_call", "location": {"end": {"column": 26, "line": 6}, "start": {"column": 3, "line": 6}}, "mutatorName": "{{.*}}", "replacement": "", {{.*}} "status": "Killed"}]{{.*}} "framework": {"brandingInformation": {"homepageUrl": "https://github.com/mull-project/mull"}, "name": "Mull", "version": "{{.*}}, LLVM {{.*}}"}{{.*}}

CHECK:{{^.*}}Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:6:3: warning: Killed: Removed the call to the function [cxx_remove_void_call]
CHECK:  void_sum(a, b, &result);
CHECK:  ^
CHECK:{{^.*}}Mutation score: 100%

RUN: cat Output/sample-ir2.txt | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NEW
CHECK-NEW:{{^.*}}Killed mutants (1/1):
CHECK-NEW:{{^.*}}sample.cpp:6:3: warning: Killed: Replaced void_sum(a, b, &result) with  [cxx_remove_void_call]
CHECK-NEW:  void_sum(a, b, &result);
CHECK-NEW:  ^
CHECK-NEW:{{^.*}}Mutation score: 100%
**/
