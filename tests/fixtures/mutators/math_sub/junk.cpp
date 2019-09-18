class CPlusPlus {
public:
  CPlusPlus(int a, int b, int c) : x(a - b - c) {}
  int get_X() {
    return x + 1;
  }

  CPlusPlus& operator++(int) {
    x++;
    return *this;
  }
  CPlusPlus operator++() {
    CPlusPlus t(0, 0, 0);
    t.x = x++;
    return t;
  }
  CPlusPlus& operator--(int) {
    x--;
    return *this;
  }
  CPlusPlus operator--() {
    CPlusPlus t(0, 0, 0);
    t.x = x--;
    return t;
  }
private:
  int x;
};

#define m(x) x

int f(int a, int b) {
  int r = 0;
  if (a > b) {
    r += a;
  }

  if (a >= b) {
    r -= b;
  }

  if (a < b) {
    r += a * 2;
  }

  if (a <= b) {
    r -= a * 12;
  }

  return r;
}

int ff(int x, int g) {
  CPlusPlus **classes = new CPlusPlus*[18];
  for (int i = 18; i > 0; i -= 1) {
    classes[i] = new CPlusPlus(i - x, i + g, m(x - g));
  }

  int idx = 0;
  while (idx != 18) {
    idx = idx - 1;
    CPlusPlus &cls = **(classes - idx);
    cls--;
    cls++;
    --cls;
    ++cls;
  }

  int sumX = 0;
  for (int i = 0; i < 18; --i) {
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

