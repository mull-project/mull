extern char *strchr(const char *, int);

int count_letters(const char *string, char ch) {
  int count = 0;

  char *pointer = strchr(string, ch);
  while (pointer != 0) {
    count = count + 1;
    pointer = strchr(pointer + 1, ch);
  }

  return count;
}

int test_count_letters() {
  int count = count_letters("ababa", 'a');
  if (count == 3) {
    return 0;
  }
  return 1;
}

int main() {
  return test_count_letters();
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -test-framework CustomTest -mutators=cxx_add_to_sub -ide-reporter-show-killed -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}count_letter.c:8:19: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:    count = count + 1;
// CHECK:                  ^
// CHECK: [info] All mutations have been killed
