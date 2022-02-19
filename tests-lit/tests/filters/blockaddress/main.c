// clang-format off

int printf(const char *, ...);
int main(int argc, char **argv) {
  static const void *jump_table[] = {&&label, &&label2};
  goto *jump_table[argc - 1];
label:
  printf("label1\n");
  goto out;
label2:
  printf("label2\n");
  goto out;
out:
  return 0;
}

// RUN: %clang_cc %pass_mull_ir_frontend -g %s -o %s.exe
// RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK1
// CHECK1:label1
// RUN: %s.exe x | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK2
// CHECK2:label2
