struct Meters {
  int value;
};
Meters operator+(Meters a, Meters b);

int sum(int a, int b) {
  return a + b;
}

template <typename T> T twice(T a) {
  return a + a;
}

long distance(const int *begin, const int *end) {
  return end - begin;
}
