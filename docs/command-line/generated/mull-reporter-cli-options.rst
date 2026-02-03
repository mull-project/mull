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

SQLITE-REPORT		Path to the sqlite report
