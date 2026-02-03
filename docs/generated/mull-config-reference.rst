Execution
---------

``workers``
   How many threads to use

   CLI equivalent: ``--workers``

``timeout``
   Timeout per test run (milliseconds)

   CLI equivalent: ``--timeout``

Behavior
--------

``debugEnabled``
   Enables Debug Mode: more logs are printed

   CLI equivalent: ``--debug``

Runner
------

``debug.coverage``
   Print coverage ranges

   CLI equivalent: ``--debug-coverage``

``includeNotCovered``
   Include (but do not run) not covered mutants. Disabled by default

   CLI equivalent: ``--include-not-covered``

``dryRunEnabled``
   Skips mutant execution and generation. Disabled by default

   CLI equivalent: ``--dry-run``

Output
------

``quiet``
   Suppress informational messages

``silent``
   Suppress all output except errors

``captureTestOutput``
   Capture stdout/stderr from the original test run

``captureMutantOutput``
   Capture stdout/stderr from mutant test runs

Mutators
--------

``mutators``
   Mutator IDs or groups to enable

``ignoreMutators``
   Mutator IDs or groups to exclude

Junk Detection
--------------

``compilationDatabasePath``
   Path to compilation database (compile_commands.json)

``compilerFlags``
   Additional compiler flags for junk detection

``junkDetectionDisabled``
   Disable junk detection (AST-based mutant validation)

Filtering
---------

``includePaths``
   File path regex patterns to include in mutation testing

``excludePaths``
   File path regex patterns to exclude from mutation testing

``gitDiffRef``
   Git ref to diff against for incremental mutation testing

``gitProjectRoot``
   Root directory of the git project

Parallelization
---------------

``parallelization.workers``
   Number of worker threads for test discovery

``parallelization.executionWorkers``
   Number of worker threads for mutant execution

Debug
-----

``debug.printIR``
   Print LLVM IR

``debug.printIRBefore``
   Print LLVM IR before mutation

``debug.printIRAfter``
   Print LLVM IR after mutation

``debug.printIRToFile``
   Print LLVM IR to a file

``debug.traceMutants``
   Trace mutant discovery and execution

``debug.coverage``
   Enable coverage debug output

``debug.gitDiff``
   Enable git diff debug output

``debug.filters``
   Enable filter debug output

``debug.slowIrVerification``
   Enable slow IR verification
