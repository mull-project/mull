// clang-format off
int sum(int a, int b) {
  return a + b;
}

int diff(int a, int b) {
  return a - b;
}

int main() {
#if defined(TEST_BOTH)
  return sum(2, 5) == 7 && diff(7, 5) == 2 ? 0 : 1;
#elif defined(TEST_B)
  return diff(7, 5) == 2 ? 0 : 1;
#else
  return sum(2, 5) == 7 ? 0 : 1;
#endif
}

/*
a.exe kills cxx_add_to_sub only, b.exe kills cxx_sub_to_add only.

RUN: rm -rf %S/Output/cache && mkdir -p %S/Output/cache
RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -grecord-command-line -o %S/Output/cache/a.exe
RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -grecord-command-line -DTEST_B -o %S/Output/cache/b.exe

Default cache: a rerun reuses everything, survivors included, and reports the same.

RUN: cd %S/Output/cache && unset TERM; env TMPDIR=%S/Output/cache/tmp MULL_CONFIG=%S/mull.yml %mull_runner --allow-surviving --reporters SQLite --report-name first a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefixes=CHECK-DEFAULT-PATH,CHECK-FRESH --implicit-check-not=Reusing
RUN: cd %S/Output/cache && unset TERM; env TMPDIR=%S/Output/cache/tmp MULL_CONFIG=%S/mull.yml %mull_runner --allow-surviving --reporters SQLite --report-name rerun a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-ALL-CACHED --implicit-check-not="Baseline run" --implicit-check-not="Running mutants"
CHECK-DEFAULT-PATH: [info] Using cache: {{.*}}/Output/cache/tmp/mull/{{.*}}.sqlite
CHECK-EXPLICIT-PATH: [info] Using cache: shared.sqlite
CHECK-FRESH: [info] Running mutants
CHECK-ALL-CACHED: [info] Reusing 2 cached result(s)

RUN: cd %S/Output/cache && %sqlite3 first.sqlite "select mutant_id, execution_status, exit_status, stdout, stderr, mutation_replacement from mutant order by mutant_id" > first.txt
RUN: cd %S/Output/cache && %sqlite3 rerun.sqlite "select mutant_id, execution_status, exit_status, stdout, stderr, mutation_replacement from mutant order by mutant_id" > rerun.txt
RUN: cd %S/Output/cache && diff first.txt rerun.txt
RUN: cd %S/Output/cache && cat first.txt | %filecheck %s --dump-input=fail --check-prefix=CHECK-REPORT
CHECK-REPORT: cxx_add_to_sub:{{.*}}|1|1|||-
CHECK-REPORT: cxx_sub_to_add:{{.*}}|2|0|||+

Interrupted run: the missing result is run again, the report is still the same.

RUN: cd %S/Output/cache && %sqlite3 tmp/mull/*.sqlite "delete from mutant where mutant_id like 'cxx_sub_to_add:%'"
RUN: cd %S/Output/cache && unset TERM; env TMPDIR=%S/Output/cache/tmp MULL_CONFIG=%S/mull.yml %mull_runner --allow-surviving --reporters SQLite --report-name resumed a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-RESUMED
CHECK-RESUMED: [info] Reusing 1 cached result(s)
CHECK-RESUMED: [info] Running mutants
RUN: cd %S/Output/cache && %sqlite3 resumed.sqlite "select mutant_id, execution_status, exit_status, stdout, stderr, mutation_replacement from mutant order by mutant_id" > resumed.txt
RUN: cd %S/Output/cache && diff first.txt resumed.txt

A broken cache is a warning, explicit or default, and the run goes on without it.

RUN: cd %S/Output/cache && echo garbage > broken.sqlite && echo file > not-a-dir
RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path broken.sqlite --allow-surviving a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-BROKEN
RUN: cd %S/Output/cache && unset TERM; env TMPDIR=%S/Output/cache/not-a-dir MULL_CONFIG=%S/mull.yml %mull_runner --allow-surviving a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-BROKEN
CHECK-BROKEN: [warning] Cannot open cache
CHECK-BROKEN: [info] Running mutants

--no-cache skips the cache.

RUN: cd %S/Output/cache && unset TERM; env TMPDIR=%S/Output/cache/tmp MULL_CONFIG=%S/mull.yml %mull_runner --no-cache --allow-surviving a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-FRESH --implicit-check-not=Reusing --implicit-check-not='Using cache'

Shared cache: kills carry over between programs, survivors are run again.

RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path shared.sqlite --allow-surviving a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefixes=CHECK-EXPLICIT-PATH,CHECK-FRESH --implicit-check-not=Reusing
RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path shared.sqlite --reporters SQLite --report-name b b.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-SHARED
CHECK-SHARED: [info] Reusing 1 cached result(s)
CHECK-SHARED: [info] Running mutants
RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path shared.sqlite --reporters SQLite --report-name a a.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-ALL-CACHED --implicit-check-not="Running mutants"
RUN: cd %S/Output/cache && %sqlite3 b.sqlite "select mutant_id, execution_status from mutant order by mutant_id" | %filecheck %s --dump-input=fail --check-prefix=CHECK-ALL-KILLED
RUN: cd %S/Output/cache && %sqlite3 a.sqlite "select mutant_id, execution_status from mutant order by mutant_id" | %filecheck %s --dump-input=fail --check-prefix=CHECK-ALL-KILLED
CHECK-ALL-KILLED: cxx_add_to_sub:{{.*}}|1
CHECK-ALL-KILLED-NEXT: cxx_sub_to_add:{{.*}}|1

Explicit cache across rebuilds: after the tests improve, kills are reused and survivors run again.

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -grecord-command-line -o %S/Output/cache/c.exe
RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path incremental.sqlite --allow-surviving c.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-FRESH --implicit-check-not=Reusing
RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path incremental.sqlite --allow-surviving c.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-ALL-CACHED --implicit-check-not="Running mutants" --implicit-check-not=outdated
RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -grecord-command-line -DTEST_BOTH -o %S/Output/cache/c.exe
RUN: cd %S/Output/cache && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --cache-path incremental.sqlite --reporters SQLite --report-name improved c.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-IMPROVED
CHECK-IMPROVED: [info] Reusing 1 cached result(s), 1 outdated by a rebuild
CHECK-IMPROVED: [info] Running mutants
RUN: cd %S/Output/cache && %sqlite3 improved.sqlite "select mutant_id, execution_status from mutant order by mutant_id" | %filecheck %s --dump-input=fail --check-prefix=CHECK-ALL-KILLED
*/
