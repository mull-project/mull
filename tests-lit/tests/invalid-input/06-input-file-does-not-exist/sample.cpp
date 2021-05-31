// clang-format off

/**
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc this-file-does-not-exist 2>&1; test $? = 1) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines  --check-prefix=CHECK-CXX
CHECK-CXX:[error] The provided path to an executable program is not valid: this-file-does-not-exist
CHECK-CXX:[error] Error messages are treated as fatal errors. Exiting now.

RUN: (%mull_runner this-file-does-not-exist 2>&1; test $? = 1) | %FILECHECK_EXEC %s --dump-input=fail --match-full-lines --check-prefix=CHECK-RUNNER
CHECK-RUNNER:[error] The provided path to an executable program is not valid: this-file-does-not-exist
CHECK-RUNNER:[error] Error messages are treated as fatal errors. Exiting now.
*/
