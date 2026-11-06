#define CHECK(cond) ((cond) ? 0 : 1)
int main(int argc, char **) {
  return CHECK(argc + 1 == 2);
}
