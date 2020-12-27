extern int printf(const char *, ...);
extern int LevenshteinDistance(const char *s, int len_s, const char *t, int len_t);

const int fail = 1;
const int success = 0;

int passing_test() {
  int actual = 0;
  int expected = 0;

  actual = LevenshteinDistance("industry", 9, "interest", 9);
  expected = 6;

  if (actual != expected) {
    printf("test failed: expected %d, got %d\n", expected, actual);
    return fail;
  }

  actual = LevenshteinDistance("horse", 6, "ros", 4);
  expected = 3;
  if (actual != expected) {
    printf("test failed: expected %d, got %d\n", expected, actual);
    return fail;
  }

  return success;
}
