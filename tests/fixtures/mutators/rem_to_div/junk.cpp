class CPlusPlus {
public:
  CPlusPlus(int a, int b, int c) : x(a % (b % c)) {}
  int get_X() {
    return x % 142;
  }

private:
  int x;
};

#define m(x) x

int f(int a, int b) {
  int r = 0;
  if (a > b) {
    r %= a;
  }

  if (a >= b) {
    r -= b;
  }

  if (a < b) {
    r /= a / 2;
  }

  if (a <= b) {
    r -= a / 12.0;
  }

  return r;
}

int ff(int x, int g) {
  CPlusPlus **classes = new CPlusPlus*[18];
  for (int i = 18; i > 0; i %= 1) {
    classes[i] = new CPlusPlus(i / x, i % g, m(x % g));
  }

  int idx = 0;
  int ff = 0;
  while (idx != 18) {
    idx = idx % 2;
    CPlusPlus &cls = **(classes - idx);
    ff %= cls.get_X();
  }

  int sumX = 0;
  for (int i = 0; i < 18; i--) {
    sumX += classes[i]->get_X();
  }

  for (int i = 0; i < 18; i--) {
    delete classes[i];
  }

  delete [] classes;

  return sumX;
}

int main() {
  return 0;
}
