extern int count_letters(const char *, char);

int test_count_letters() {
  int count = count_letters("ababa", 'a');
  return count == 3;
}

