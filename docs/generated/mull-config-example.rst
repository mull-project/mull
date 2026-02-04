Full configuration example
-------------------------

.. code-block:: yaml

   # Timeout per test run (milliseconds)
   timeout: 3000
   # Enables Debug Mode: more logs are printed
   debugEnabled: false
   # Include (but do not run) not covered mutants. Disabled by default
   includeNotCovered: false
   # Skips mutant execution and generation. Disabled by default
   dryRunEnabled: false
   # Suppress informational messages
   quiet: true
   # Suppress all output except errors
   silent: false
   # Capture stdout/stderr from the original test run
   captureTestOutput: true
   # Capture stdout/stderr from mutant test runs
   captureMutantOutput: true
   # Mutator IDs or groups to enable
   mutators: []
   # Mutator IDs or groups to exclude
   ignoreMutators: []
   # Path to compilation database (compile_commands.json)
   compilationDatabasePath: ""
   # Additional compiler flags for junk detection
   compilerFlags: []
   # Disable junk detection (AST-based mutant validation)
   junkDetectionDisabled: false
   # File path regex patterns to include in mutation testing
   includePaths: []
   # File path regex patterns to exclude from mutation testing
   excludePaths: []
   # Git ref to diff against for incremental mutation testing
   gitDiffRef: ""
   # Root directory of the git project
   gitProjectRoot: ""
   parallelization:
     # Number of worker threads for test discovery
     workers: 0
     # Number of worker threads for mutant execution
     executionWorkers: 0
   debug:
     # Print LLVM IR
     printIR: false
     # Print LLVM IR before mutation
     printIRBefore: false
     # Print LLVM IR after mutation
     printIRAfter: false
     # Print LLVM IR to a file
     printIRToFile: false
     # Trace mutant discovery and execution
     traceMutants: false
     # Enable coverage debug output
     coverage: false
     # Enable git diff debug output
     gitDiff: false
     # Enable filter debug output
     filters: false
     # Enable slow IR verification
     slowIrVerification: false
