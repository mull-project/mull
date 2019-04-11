#define ff(x, y) ( (x) < (y) )

extern "C" void buzz(int x);

void bar(bool x) {
  if (x) {
    buzz(18);
  } else {
    buzz(42);
  }
}

template<typename T>
T foo(T a, T b) {
  if (ff(a, b)) {
    return a;
  }

  if (ff(b, a)) {
    return b;
  }

  if (a == b) {
    return a;
  }
  if (a != b) {
    return b;
  }

  if (a > b) {
    return a;
  }

  if (a < b) {
    return b;
  }

  if (a >= b) {
    return b;
  }

  if (a <= b) {
    return a;
  }

  if (a) {
    return b;
  }

  if (b) {
    return a;
  }

  if (!a) {
    return a;
  }

  if (!b) {
    return b;
  }

  bar(a > b);
  bar(a != b);

  return a;
}

int f_unsigned_int() {
  unsigned int a = 42;
  unsigned int b = 33;
  return foo(a, b);
}

int f_signed_int() {
  signed int a = 42;
  signed int b = 33;
  return foo(a, b);
}

int f_float() {
  float a = 42;
  float b = 33;
  return foo(a, b);
}

