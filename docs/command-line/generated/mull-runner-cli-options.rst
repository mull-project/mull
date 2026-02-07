--test-program PATH		Path to a test program (if different from input executable)

--workers N		Number of parallel workers for mutation search

--timeout MS		Timeout per test run in milliseconds

--include-not-covered		Include mutants on lines not covered by tests

--dry-run		Skip mutant execution, only discover and report mutants

--ld-search-path DIR		Library search path

--coverage-info PATH		Path to the coverage info file (LLVM profdata)

--debug-coverage		Print coverage ranges

--reporters REPORTER		Output reporters to use (IDE, SQLite, Elements, Patches, GithubAnnotations)

--report-dir DIR		Directory for report output files

--report-name NAME		Filename for the report (only for supported reporters)

--report-patch-base DIR		Base directory for patch file paths

--ide-reporter-show-killed		Show killed mutations in IDE reporter output

--debug		Enable debug mode with additional diagnostic output

--strict		Treat warnings as fatal errors

--allow-surviving		Do not treat surviving mutants as an error

--mutation-score-threshold SCORE		Minimum mutation score (0-100) required for success

--no-test-output		Does not capture output from test runs

--no-mutant-output		Does not capture output from mutant runs

--no-output		Combines --no-test-output and --no-mutant-output

EXECUTABLE		Path to the executable under test

RUNNER_ARGS		Arguments passed to the test runner
