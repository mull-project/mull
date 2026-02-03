.. code-block:: yaml

   # Timeout per test run in milliseconds. DEPRECATED, use CLI flag instead.
   timeout: 3000
   # Enable debug mode with additional diagnostic output.
   debugEnabled: false
   # Include mutants on lines not covered by tests. DEPRECATED, use CLI flag instead.
   includeNotCovered: false
   # Skip mutant execution, only discover and report mutants. DEPRECATED, use CLI flag instead.
   dryRunEnabled: false
   # Suppress informational messages (enabled by default).
   quiet: true
   # Suppress all output except errors.
   silent: false
   # Turn warnings into errors.
   strict: false
   # Capture stdout/stderr from the original test run (enabled by default).
   captureTestOutput: true
   # Capture stdout/stderr from mutant test runs (enabled by default).
   captureMutantOutput: true
   # Mutator IDs or groups to enable (e.g. cxx_add_to_sub, cxx_logical).
   mutators: []
   # Mutator IDs or groups to exclude (e.g. cxx_add_to_sub, cxx_logical).
   ignoreMutators: []
   # Path to compilation database (compile_commands.json).
   compilationDatabasePath: ""
   # Additional compiler flags for junk detection.
   compilerFlags: []
   # Disable junk detection (AST-based mutant validation).
   junkDetectionDisabled: false
   # File path regex patterns to include in mutation testing.
   includePaths: []
   # File path regex patterns to exclude from mutation testing.
   excludePaths: []
   # Git ref to diff against for incremental mutation testing.
   gitDiffRef: ""
   # Root directory of the git project.
   gitProjectRoot: ""
   # Parallelization settings
   parallelization:
     # Number of worker threads for test discovery.
     workers: $nproc
     # Number of worker threads for mutant execution.
     executionWorkers: $nproc
   # Advanced debug options for diagnostic output.
   debug:
     # Print LLVM IR.
     printIR: false
     # Print LLVM IR before mutation.
     printIRBefore: false
     # Print LLVM IR after mutation.
     printIRAfter: false
     # Print LLVM IR to a file.
     printIRToFile: false
     # Trace mutant discovery and execution.
     traceMutants: false
     # Enable coverage debug output.
     coverage: false
     # Enable git diff debug output.
     gitDiff: false
     # Enable filter debug output.
     filters: false
     # Enable slow IR verification.
     slowIRVerification: false
