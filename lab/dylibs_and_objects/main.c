//extern int printf(const char *, ...);
//extern int strcmp(const char *, const char *);

#include <stdio.h>
#include <string.h>

extern int fail;
extern int success;

extern int passing_test();
extern int failing_test();

static int GlobalVariable = 15;

__attribute__((constructor)) static void initGlobalVariable() {
  GlobalVariable = 42;
}

int main(int argc, char **argv) {
  void *p = &strcmp;
  if (argc == 1) {
    printf("no test specified\n");
    return 48;
  }

  if (argc != 2) {
    printf("wrong number of parameters specified\n");
    return 93;
  }

  const char *testName = argv[1];

  /* printf("%p\n", &strcmp); */

  if (strcmp(testName, "passing_test") == 0) {
    int result = passing_test();
    if (result == fail) {
      printf("'%s' did fail\n", testName);
      return 112;
    }
  } else if (strcmp(testName, "failing_test") == 0 && GlobalVariable == 42) {
    int result = failing_test();
    if (result == fail) {
      printf("'%s' did fail\n", testName);
      return 112;
    }
  } else {
    printf("unknown test: %s\n", testName);
    return 120;
  }

  printf("did not fail\n");

  return 0;
}
