// clang-format off

/**
RUN: (%mull_runner this-file-does-not-exist 2>&1; test $? = 1) | %filecheck %s --dump-input=fail --match-full-lines
CHECK:[error] The provided path to an executable program is not valid: this-file-does-not-exist
CHECK:[error] Error messages are treated as fatal errors. Exiting now.
*/
