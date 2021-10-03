int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
RUN: sed -e "s:%PWD:%s:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR

RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -mutators=cxx_add_to_sub -reporters=Elements "%s.exe" 2>&1; test $? = 0) | %filecheck "%s" --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-COMPDB-PROVIDED
WITHOUT-COMPDB-PROVIDED:[warning] Mutation Testing Elements Reporter may not work without compilation database. Consider providing -compdb-path or -compilation-flags.

RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -mutators=cxx_add_to_sub -reporters=Elements -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json "%s.exe" 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-COMPDB-PROVIDED
WITH-COMPDB-PROVIDED:{{^.*}}[info] Running mutants (threads: 1){{$}}
WITH-COMPDB-PROVIDED:[info] Killed mutants (1/1):
WITH-COMPDB-PROVIDED:{{^.*}}/sample.cpp:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
WITH-COMPDB-PROVIDED:  return a + b;
WITH-COMPDB-PROVIDED:           ^
WITH-COMPDB-PROVIDED:[info] All mutations have been killed
WITH-COMPDB-PROVIDED:[info] Mutation score: 100%
**/
