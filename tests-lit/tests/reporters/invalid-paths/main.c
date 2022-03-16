int add(int a, int b) {
  return a + b;
}

int main() {
  add(2, 2);
  return 0;
}

// clang-format off
/**
RUN: cp %s Output/main.c
RUN: %clang_cc %sysroot -g -O0 %pass_mull_ir_frontend Output/main.c -o Output/main.exe
RUN: rm Output/main.c
RUN: %mull_runner Output/main.exe -reporters IDE -reporters Patches -reporters Elements | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[warning] IDEReporter: Cannot report 'cxx_add_to_sub:{{.*}}/Output/main.c:2:12': cannot read {{.*}}Output/main.c
CHECK:[warning] PatchesReporter: Cannot report 'cxx_add_to_sub:{{.*}}/Output/main.c:2:12': cannot read {{.*}}Output/main.c
CHECK:[warning] ElementsReporter: Cannot report 'cxx_add_to_sub:{{.*}}/Output/main.c:2:12': cannot read {{.*}}Output/main.c
*/
