extern int printf(const char *fmt, ...);

typedef enum ST_Result {
  ST_Success = 1,
  ST_Fail = 2
} ST_Result;

typedef ST_Result (*st_test_ptr)();

#define st_assert(x) (x) ? ST_Success : ST_Fail

/// Prototypes

int addition(int a, int b);

/// Tests

ST_Result test_addition_5_plus_6_should_be_greater_than_zero() {
  int a = 5;
  int b = 6;

  return st_assert(addition(a, b) > 0);
}

ST_Result test_addition_minus_16_plus_minus_2_should_be_minus_18() {
  int a = -16;
  int b = -2;

  return st_assert(addition(a, b) == -18);
}

ST_Result test_addition_minus_8_plus_6_should_be_minus_2() {
  int a = -8;
  int b = 6;

  return st_assert(addition(a, b) == -2);
}

ST_Result test_addition_minus_8_plus_6_should_be_2() {
  int a = -8;
  int b = 6;

  return st_assert(addition(a, b) == 2);
}

/// Implementations

int addition(int a, int b) {
  return a + b;
}

/// Driver

int main() {
  st_test_ptr tests[] = { 
    test_addition_5_plus_6_should_be_greater_than_zero,
    test_addition_minus_16_plus_minus_2_should_be_minus_18,
    test_addition_minus_8_plus_6_should_be_minus_2,
    test_addition_minus_8_plus_6_should_be_2
  };
  int tests_count = sizeof(tests) / sizeof(st_test_ptr);

  int tests_failed = 0;
  for (int i = 0; i < tests_count; i++) {
    ST_Result result = tests[i]();
    if (result != ST_Success) {
      printf("F");
    } else {
      printf(".");
    }
  }

  printf("\n");

  return tests_failed;
}

