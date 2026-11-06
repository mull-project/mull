struct vec {
  int size = 0;
  void resize(int n) {
    size = n;
  }
};
int main() {
  vec v;
  v.resize(2);
  return v.size != 2;
}
