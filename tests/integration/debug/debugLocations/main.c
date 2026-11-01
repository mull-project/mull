// clang-format off

int sum(int a, int b) {
  return a
         + b;
}

int main() {
  return sum(1, -1);
}

// RUN: %clang_cc %sysroot %pass_mull_ir_frontend -g -S -emit-llvm %s -o - 2>/dev/null | %filecheck %s --dump-input=fail
// CHECK: define {{.*}}@sum({{.*}} !dbg ![[TRAMPOLINE:[0-9]+]] {
// CHECK: call {{.*}}, !dbg ![[TRAMPOLINE_LOC:[0-9]+]]
// CHECK: define {{.*}}cxx_add_to_sub{{.*}} !dbg ![[MUTANT:[0-9]+]] {
// CHECK: sub {{.*}}, !dbg ![[MUTATION_LOC:[0-9]+]]
// CHECK: ![[TRAMPOLINE]] = distinct !DISubprogram(name: "sum"
// CHECK: ![[TRAMPOLINE_LOC]] = !DILocation(line: 3, scope: ![[TRAMPOLINE]])
// CHECK: ![[MUTANT]] = distinct !DISubprogram(name: "sum"
// CHECK: ![[MUTATION_LOC]] = !DILocation(line: 5, column: 10, scope: ![[MUTANT]])
