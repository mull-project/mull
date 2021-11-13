int equal(int a, int b) {
  return a == b;
}

int main() {
  equal(2, 2);
  return 0;
}

// clang-format off
/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -fembed-bitcode -g -O0 Output/sandbox/main.cpp -o Output/main.cpp.exe

RUN: cd %S/Output && echo $PATH; (unset TERM; %mull_cxx -mutators=cxx_eq_to_ne -linker-flags="%sysroot" --linker=%clang_cxx -debug main.cpp.exe --report-name test --reporters GithubAnnotations --reporters IDE; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:\::warning file={{.*}}/main.cpp,line=2,col=12,endLine=2,endColumn=14::[cxx_eq_to_ne] Replaced == with !=
*/
