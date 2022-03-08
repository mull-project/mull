extern "C" long write(int fildes, const void *buf, long nbyte);

int equal(int a, int b) {
  return a == b;
}

int main() {
  write(1, "stdout", 7);
  write(2, "stderr", 7);
  equal(2, 2);
  return 0;
}

// clang-format off
/**
RUN: %clang_cxx %sysroot -g -O0 %pass_mull_ir_frontend %s -o Output/main.cpp.exe
RUN: rm -f test.sqlite
RUN: %mull_runner Output/main.cpp.exe --report-name test --reporters SQLite
RUN: sqlite3 ./test.sqlite -line "select * from information order by key" | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-INFO
CHECK-INFO:  key = Build Date
CHECK-INFO:  key = Commit
CHECK-INFO:  key = LLVM Version
CHECK-INFO:  key = Mull Version

RUN: sqlite3 ./test.sqlite -line "select * from mutant" | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTANT
CHECK-MUTANT:    mutant_id = cxx_eq_to_ne:{{.*}}
CHECK-MUTANT:      mutator = cxx_eq_to_ne
CHECK-MUTANT:     filename = {{.*}}main.cpp
CHECK-MUTANT:    directory =
CHECK-MUTANT:  line_number = 4
CHECK-MUTANT:column_number = 12
CHECK-MUTANT:       status = 2
CHECK-MUTANT:     duration = {{.*}}
CHECK-MUTANT:       stdout = stdout
CHECK-MUTANT:       stderr = stderr
*/
