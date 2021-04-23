extern int printf(const char *, ...);
extern int strcmp(const char *, const char *);

int test1(int a, int b) {
  return a + b;
}

int test2(int a, int b) {
  return a * b;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("NOT ENOUGH ARGUMENTS\n");
    return 1;
  }
  if (strcmp(argv[1], "first test") == 0) {
    if (test1(2, 5) == 7) {
      printf("first test passed\n");
      return 0;
    } else {
      printf("first test failed\n");
      return 1;
    }
  } else if (strcmp(argv[1], "second test") == 0) {
    if (test2(2, 5) == 10) {
      printf("second test passed\n");
      return 0;
    } else {
      printf("second test failed\n");
      return 1;
    }
  } else {
    printf("INCORRECT TEST NAME %s\n", argv[1]);
    return 1;
  }
  return 0;
}

// clang-format off

// RUN: %clang_cc %s -fembed-bitcode -g -o %s.exe

// RUN: %mull_cxx -linker=%clang_cc -keep-executable -mutate-only -output=%s.mutated.exe -mutators=cxx_add_to_sub -mutators=cxx_mul_to_div %s.exe | %FILECHECK_EXEC %s --dump-input=fail --match-full-lines --check-prefix=CHECK-MUTATE
// CHECK-MUTATE: [info] Mutate-only mode on:{{.*}}
// CHECK-MUTATE-NOT: [info] Sanity check run{{.*}}
// CHECK-MUTATE-NOT: Running mutants

// RUN: %mull_runner %s.mutated.exe -ide-reporter-show-killed -test-program %python3 -- %S/test.py %s.mutated.exe "first test case" | %FILECHECK_EXEC %s --dump-input=fail --match-full-lines --check-prefix=CHECK-TEST1
// CHECK-TEST1: [info] Killed mutants (1/2):
// CHECK-TEST1: {{.*}}/main.c:5:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK-TEST1:   return a + b;
// CHECK-TEST1:            ^
// CHECK-TEST1: [info] Survived mutants (1/2):
// CHECK-TEST1: {{.*}}/main.c:9:12: warning: Survived: Replaced * with / [cxx_mul_to_div]
// CHECK-TEST1:   return a * b;
// CHECK-TEST1:            ^

// RUN: %mull_runner %s.mutated.exe -ide-reporter-show-killed -test-program %python3 -- %S/test.py %s.mutated.exe "second test case" | %FILECHECK_EXEC %s --dump-input=fail --match-full-lines --check-prefix=CHECK-TEST2
// CHECK-TEST2: [info] Killed mutants (1/2):
// CHECK-TEST2: {{.*}}/main.c:9:12: warning: Killed: Replaced * with / [cxx_mul_to_div]
// CHECK-TEST2:   return a * b;
// CHECK-TEST2:            ^
// CHECK-TEST2: [info] Survived mutants (1/2):
// CHECK-TEST2: {{.*}}/main.c:5:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-TEST2:   return a + b;
// CHECK-TEST2:            ^
