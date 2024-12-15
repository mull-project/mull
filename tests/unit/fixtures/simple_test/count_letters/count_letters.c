extern char *strchr(const char *, int);
extern int printf(const char *, ...);

int count_letters(const char *string, char ch) {
  int count = 0;

  char *pointer = strchr(string, ch);
  while (pointer != 0) {
    count = count + 1;
    pointer = strchr(pointer + 1, ch);
  }

  return count;
}

