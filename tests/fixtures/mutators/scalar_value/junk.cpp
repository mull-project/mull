int main(int argc, char **argv) { return argc + 42 + 122.f; }

int f1(int x) {
  const int C = 136;
  const int X = C;
  return x + X;
}

const int C = 115;

int f2(int x) {
  const int X = C;
  return x + X;
}

int f3(int x) {
  const int A = C;
  const int B = 220;
  const int X = A + B;
  return x + X;
}