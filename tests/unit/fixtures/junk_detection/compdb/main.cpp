#include "header.h"

int f(int a, int b) {
  int r = 0;
  if (a > b) {
    r += a;
  }

  if (a >= b) {
    r += b;
  }

  if (a < b) {
    r += a * 2;
  }

  if (a <= b) {
    r += a * 12;
  }

  return r;
}

int ff(int x, int g) {
  int *mem = new int[x];

  mem[0] = f(x, g);

  for (int i = 1; i < x; i++) {
    mem[i] = f(mem[i - 1], x);
  }

  int r = 0;
  for (int i = 0; i < x; i++) {
    r += mem[i];
  }

  delete [] mem;

  return r;
}

int fff(int a, int b) {
  if (m(a > b)) {
    return a;
  }

  return b;
}

int main() {
  CPlusPlus cpp;
  return cpp.foobar(44);
}



