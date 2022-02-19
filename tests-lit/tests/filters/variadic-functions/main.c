// clang-format off
#include <stdarg.h>

extern unsigned long strlen(const char *s);
extern char *strcpy(char * dst, const char * src);
extern int printf(const char *, ...);
extern void abort();

static int a = 10;
static int b = 32;
static int c = 0;

void fill_in(const char *what, int n, ...) {
  c = a + b;
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; ++i) {
    char *buf = va_arg(args, char *);
    int w_len = strlen(what);
    int b_len = strlen(buf);
    if (w_len > b_len) {
      printf("CRASHING\n");
      abort();
    }
    strcpy(buf, what);
    buf[i] = '_';
    buf[b_len] = '\0';
  }
  va_end(args);
}

int main() {
  char b1[16] = "initialize";
  char b2[16] = "initialize";
  char b3[16] = "initialize";
  char b4[16] = "initialize";
  fill_in("test", 4, b1, b2, b3, b4);
  printf("%s\n", b1);
  printf("%s\n", b2);
  printf("%s\n", b3);
  printf("%s\n", b4);
  printf("%d\n", c);
  return 0;
}

// RUN: %clang_cc %pass_mull_ir_frontend -g %s -o %s.exe
// RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:_est
// CHECK-NEXT:t_st
// CHECK-NEXT:te_t
// CHECK-NEXT:tes_
// CHECK-NEXT:42
// CHECK-EMPTY

// RUN: unset TERM; %mull_runner %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK_MUTANT
// CHECK_MUTANT:[info] No mutants found. Mutation score: infinitely high
