#define m(x) x

int f(int a, int b) {
  int r = 0;
  if (a && b) {
    r += a;
  }

  if (a || b) {
    r -= b;
  }

  if (a and b) {
    r += a * 2;
  }

  if (a or b) {
    r -= a * 12;
  }

  if (m(a && b)) {
    r -= a * 12;
  }

  if (m(a or b)) {
    r -= a * 12;
  }

  return r;
}

int main() {
  return 0;
}
