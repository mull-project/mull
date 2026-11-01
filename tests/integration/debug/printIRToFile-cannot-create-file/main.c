int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(2, 5) == 7 ? 0 : 1;
}

// clang-format off

/**
A directory is in the way of the IR files, so they cannot be created. That must not stop the compilation.

RUN: mkdir -p %s.before.ll %s.after.ll
RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %pass_mull_ir_frontend -g -grecord-command-line %s -o %s.exe 2>&1 | %filecheck %s --dump-input=fail

CHECK:[warning] Cannot create file for debugging: {{.*}}main.c.before.ll: {{.*}}
CHECK:[warning] Cannot create file for debugging: {{.*}}main.c.after.ll: {{.*}}
**/
