int main() {
  return 0;
}

// clang-format off

/**
RUN: cd %S && %clang_cxx %sysroot %pass_mull_ir_frontend %s -o %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[warning] Mull cannot find config (mull.yml). Using some defaults.
CHECK:[warning] Mull cannot find debug information. Recompile with `-g` flag.
CHECK:[warning] Mull cannot find compiler flags. Recompile with `-grecord-command-line` flag.
**/


