extern int count_letters(const char *, char);

enum { SUCCESS = 0, FAILURE = 1 };

int test_count_letters() {
  int count = count_letters("ababa", 'a');
  if (count == 3) {
    return SUCCESS;
  }
  return FAILURE;
}

int main() {
  return test_count_letters();
}
