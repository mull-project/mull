struct buffer {
  int size = 0;
  char data[8];
  __attribute__((noinline)) void push_back(const char &c) {
    data[size++] = c;
  }
};
int main() {
  buffer b;
  b.push_back('a');
  return b.size != 1;
}
