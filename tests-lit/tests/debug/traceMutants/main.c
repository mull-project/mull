// clang-format off

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(0, 0);
}

// RUN: %clang_cc %pass_mull_ir_frontend -g %s -o %s.exe
// RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK
// RUN: env "cxx_add_to_sub:%s:4:12=1" %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTANT

// CHECK:mull-trace: entering sum
// CHECK:mull-trace: checking for cxx_add_to_sub:{{.*}}/main.c:4:12
// CHECK:mull-trace: jumping over to original sum
// CHECK:mull-trace: trampoline call sum

// CHECK-MUTANT:mull-trace: entering sum
// CHECK-MUTANT:mull-trace: checking for cxx_add_to_sub:{{.*}}/main.c:4:12
// CHECK-MUTANT:mull-trace: jumping over to cxx_add_to_sub:{{.*}}/main.c:4:12
// CHECK-MUTANT:mull-trace: trampoline call sum
