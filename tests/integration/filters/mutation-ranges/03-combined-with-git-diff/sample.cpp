int add(int x, int y) {
  return x + y;
}
int sub(int x, int y) {
  return x - y;
}
int mul(int x, int y) {
  return x + y;
}
int main() {
  return add(1, 2) == 3 && sub(3, 1) == 2 && mul(1, 2) == 3 ? 0 : 1;
}
