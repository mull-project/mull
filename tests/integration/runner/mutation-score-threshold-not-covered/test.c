int used(int a, int b) {
  return a + b;
}

int unused(int a, int b) {
  return a + b;
}

int main(int argc, char **argv) {
  if (argc > 5) {
    return unused(1, 2);
  }
  return used(2, 5) == 7 ? 0 : 1;
}

// clang-format off

/*
Nothing survives here, but the score is 50% because of the not covered mutant.

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %S/test.exe

RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --include-not-covered %S/test.exe 2>&1; test $? = 1
RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --include-not-covered --mutation-score-threshold 51 %S/test.exe 2>&1; test $? = 1
RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --include-not-covered --mutation-score-threshold 50 %S/test.exe
RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --include-not-covered --allow-surviving %S/test.exe
*/
