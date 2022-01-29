// clang-format off

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(0, 0);
}

// RUN: %clang_cc %pass_mull_ir_frontend -g %s -o %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:define{{.*}}@sum{{.*}}
// CHECK:define{{.*}}cxx_add_to_sub{{.*}}
