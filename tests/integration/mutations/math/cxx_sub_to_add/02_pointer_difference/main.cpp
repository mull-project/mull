long distance(const char *p, const char *q) {
  return p - q;
}
int sub(int a, int b) {
  return a - b;
}
int main() {
  const char s[] = "abc";
  return distance(s + 2, s) != 2 || sub(3, 1) != 2;
}
