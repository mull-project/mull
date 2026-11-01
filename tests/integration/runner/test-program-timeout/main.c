long count(long n) {
  long i = 0;
  while (i < n) {
    i++;
  }
  return i;
}

int main() {
  return count(3) == 3 ? 0 : 1;
}

// clang-format off

/**
The test binary is a grandchild of the runner here. It keeps running and holds the output pipes open
after the wrapper is killed, so a timed out mutant must take the whole process group down.

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %pass_mull_ir_frontend -g %s -o %s.exe
RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --minimum-timeout 200 --ide-reporter-show-killed --test-program sh %s.exe -- %S/wrapper.sh %s.exe 2>&1 | %filecheck %s --dump-input=fail

CHECK:[info] Killed mutants (1/1):
CHECK:{{.*}}main.c:4:6: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
CHECK:[info] Mutation score: 100%
**/
