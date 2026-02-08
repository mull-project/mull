.. code-block:: text

   Mull: mutation testing report generator

   Usage: mull-reporter [OPTIONS] <SQLITE-REPORT>

   Arguments:
     <SQLITE-REPORT>
             Path to the sqlite report

   Options:
         --reporters <REPORTER>
             Output reporters to use, more than one can be used at the same time

             Possible values:
             - IDE:               IDE-friendly output with file:line:column format
             - SQLite:            SQLite database for offline analysis
             - GitHubAnnotations: GitHub Actions annotation format
             - Patches:           Generate patch files for each mutation
             - Elements:          Mutation Testing Elements JSON/HTML report

         --report-dir <DIR>
             Directory for report output files

             [default: .]

         --report-name <NAME>
             Filename for the report (only for supported reporters)

         --report-patch-base <DIR>
             Base directory for patch file paths

             [default: .]

         --ide-reporter-show-killed
             Show killed mutations in IDE reporter output

         --debug
             Enable debug mode with additional diagnostic output

         --strict
             Treat warnings as fatal errors

         --allow-surviving
             Do not treat surviving mutants as an error

         --mutation-score-threshold <SCORE>
             Minimum mutation score (0-100) required for success

             [default: 100]

         --no-test-output
             Does not capture output from test runs

         --no-mutant-output
             Does not capture output from mutant runs

         --no-output
             Combines --no-test-output and --no-mutant-output

     -h, --help
             Print help (see a summary with '-h')

     -V, --version
             Print version
