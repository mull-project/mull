int equal(int a, int b) {
  return a == b;
}

int main() {
  equal(2, 2);
  return 0;
}

// clang-format off
/**

RUN: %clang_cxx %sysroot -g -O0 %pass_mull_ir_frontend main.cpp -o main.cpp-ir.exe

RUN: %mull_runner --allow-surviving main.cpp-ir.exe --report-name test --reporters Sarif --reporters SQLite | %filecheck %s --dump-input=fail --check-prefix=CHECK-RUNNER
RUN: %mull_reporter --allow-surviving test.sqlite --report-name test2 --reporters Sarif | %filecheck %s --dump-input=fail --check-prefix=CHECK-REPORTER

RUN: [[ -f %S/test.sarif ]]
RUN: %jq -r '[.version, .runs[0].tool.driver.name, .runs[0].results[0].ruleId, .runs[0].results[0].level, (.runs[0].results[0].locations[0].physicalLocation.region.startLine | tostring), (.runs[0].results[0].partialFingerprints | to_entries[0].key), (.runs[0].results[0].partialFingerprints | to_entries[0].value)] | .[]' %S/test.sarif | %filecheck %s --check-prefix=CHECK-SARIF
RUN: %jq -r '[.runs[0].tool.driver.rules[0].shortDescription.text, .runs[0].results[0].message.text] | .[]' %S/test.sarif | %filecheck %s --check-prefix=CHECK-RULE

RUN: [[ -f %S/test2.sarif ]]
RUN: %jq -r '[.version, .runs[0].results[0].ruleId] | .[]' %S/test2.sarif | %filecheck %s --check-prefix=CHECK-SARIF2

CHECK-RUNNER:[info] SARIF reporter: generated report to {{.*}}/test.sarif
CHECK-REPORTER:[info] SARIF reporter: generated report to {{.*}}/test2.sarif

CHECK-SARIF: 2.1.0
CHECK-SARIF: Mull
CHECK-SARIF: cxx_eq_to_ne
CHECK-SARIF: warning
CHECK-SARIF: 2
CHECK-SARIF: primaryLocationLineHash
CHECK-SARIF: cxx_eq_to_ne:{{.*}}:2:

CHECK-RULE: Replaces `==` with `!=`
CHECK-RULE: Replaces `==` with `!=`

CHECK-SARIF2: 2.1.0
CHECK-SARIF2: cxx_eq_to_ne
*/
