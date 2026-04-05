int equal(int a, int b) {
  return a == b;
}

int main() {
  equal(2, 2);
  return 0;
}

// clang-format off
/**

RUN: %clang_cxx %sysroot -g -O0 %pass_mull_ir_frontend %s -o %s-ir.exe

RUN: %mull_runner --allow-surviving %s-ir.exe --report-name test --reporters Sarif --report-patch-base %S | %filecheck %s --dump-input=fail --check-prefix=CHECK-RUNNER

RUN: [[ -f %S/test.sarif ]]
RUN: %jq -r '.runs[0].originalUriBaseIds | to_entries[0] | [.key, .value.uri] | .[]' %S/test.sarif | %filecheck %s --check-prefix=CHECK-SRCROOT
RUN: %jq -r '.runs[0].results[0].locations[0].physicalLocation.artifactLocation | [.uri, .uriBaseId] | .[]' %S/test.sarif | %filecheck %s --check-prefix=CHECK-LOC

CHECK-RUNNER:[info] SARIF reporter: generated report to {{.*}}/test.sarif

CHECK-SRCROOT: %SRCROOT%
CHECK-SRCROOT: file://{{.*}}/

CHECK-LOC: main.cpp
CHECK-LOC: %SRCROOT%
*/
