#include <stdio.h>

static int IntScalar = 10;
static const int ConstIntScalar = 10;

int scalar_value(int a) {
  int inlineScalar = 5;
  const int constInlineScalar = 5;

  return a * IntScalar * ConstIntScalar * inlineScalar * constInlineScalar;
}

int test_scalar_value() {
  int result = (scalar_value(4) == 10000);
  return result;
}
