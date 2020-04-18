int negate(int a) {
  return !a;
}

int double_negate(int a) {
  return !!a;
}

int main() {
  return ! (negate(15) == 0);
}
