#pragma once

#include <llvm/Support/CommandLine.h>
#include <mull/Config/Configuration.h>
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Reporters/Reporter.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// clang-format off

#define RunnerArgs_() \
list<std::string> RunnerArgs( \
    Positional, \
    desc("free form arguments"), \
    Optional, \
    value_desc("strings"), \
    ZeroOrMore, \
    cat(MullCategory))

#define TestProgram_() \
opt<std::string> TestProgram( \
    "test-program", \
    desc("Path to a test program"), \
    Optional, \
    init(""), \
    value_desc("path"), \
    cat(MullCategory))

#define InputFile_() \
opt<std::string> InputFile( \
    Positional, \
    desc("<input file>"), \
    Required, \
    value_desc("path"), \
    cat(MullCategory))

#define Workers_() \
opt<unsigned> Workers( \
    "workers", \
    desc("How many threads to use"), \
    Optional, \
    value_desc("number"), \
    cat(MullCategory)) \

#define Timeout_() \
opt<unsigned> Timeout( \
    "timeout", \
    desc("Timeout per test run (milliseconds)"), \
    Optional, \
    value_desc("number"), \
    init(MullDefaultTimeoutMilliseconds), \
    cat(MullCategory)) \

#define ReportDirectory_() \
opt<std::string> ReportDirectory( \
    "report-dir", \
    desc("Where to store report (defaults to '.')"), \
    Optional, \
    value_desc("directory"), \
    init("."), \
    cat(MullCategory))

#define ReportName_() \
opt<std::string> ReportName( \
    "report-name", \
    desc("Filename for the report (only for supported reporters). Defaults to <timestamp>.<extension>"), \
    Optional, \
    value_desc("filename"), \
    init(""), \
    cat(MullCategory))

#define ReportPatchBaseDirectory_() \
opt<std::string> ReportPatchBaseDirectory( \
    "report-patch-base", \
    desc("Create Patches relative to this directory (defaults to git-project-root if available, else absolute path will be used)"), \
    Optional, \
    value_desc("directory"), \
    init("."), \
    cat(MullCategory))

#define CoverageInfo_() \
opt<std::string> CoverageInfo( \
    "coverage-info", \
    desc("Path to the coverage info file (LLVM's profdata)"), \
    value_desc("string"), \
    Optional, \
    init(std::string()), \
    cat(MullCategory))

#define DryRunOption_() \
opt<bool> DryRunOption( \
    "dry-run", \
    desc("Skips mutant execution and generation. Disabled by default"), \
    Optional, \
    init(false), \
    cat(MullCategory))

#define IncludeNotCovered_() \
opt<bool> IncludeNotCovered( \
    "include-not-covered", \
    desc("Include (but do not run) not covered mutants. Disabled by default"), \
    Optional, \
    init(false), \
    cat(MullCategory))

#define ReportersOption_() \
list<ReporterKind> ReportersOption( \
    "reporters", \
    desc("Choose reporters:"), \
    ZeroOrMore, \
    value_desc("reporter"), \
    cat(MullCategory)) \

#define IDEReporterShowKilled_() \
opt<bool> IDEReporterShowKilled( \
    "ide-reporter-show-killed", \
    desc("Makes IDEReporter to also report killed mutations (disabled by default)"), \
    Optional, \
    init(false), \
    cat(MullCategory)) \

#define DebugEnabled_() \
opt<bool> DebugEnabled( \
    "debug", \
    desc("Enables Debug Mode: more logs are printed"), \
    Optional, \
    init(false), \
    cat(MullCategory)) \

#define StrictModeEnabled_() \
opt<bool> StrictModeEnabled( \
    "strict", \
    desc("Enables Strict Mode: all warning messages are treated as fatal errors"), \
    Optional, \
    init(false), \
    cat(MullCategory)) \

#define NoTestOutput_() \
opt<bool> NoTestOutput( \
    "no-test-output", \
    desc("Does not capture output from test runs"), \
    Optional, \
    init(false), \
    cat(MullCategory)) \

#define NoMutantOutput_() \
opt<bool> NoMutantOutput( \
    "no-mutant-output", \
    desc("Does not capture output from mutant runs"), \
    Optional, \
    init(false), \
    cat(MullCategory)) \

#define NoOutput_() \
opt<bool> NoOutput( \
    "no-output", \
    desc("Combines -no-test-output and -no-mutant-output"), \
    Optional, \
    init(false), \
    cat(MullCategory)) \

#define DumpCLIInterface_() \
opt<bool> DumpCLIInterface( \
    "dump-cli", \
    desc("Prints CLI options in the Sphinx/RST friendly format"), \
    Optional, \
    init(false), \
    cat(MullCategory), \
    Hidden) \

#define DumpMutators_() \
opt<bool> DumpMutators( \
    "dump-mutators", \
    desc("Prints available mutators in the Sphinx/RST friendly format"), \
    Optional, \
    init(false), \
    cat(MullCategory), \
    Hidden)

#define LDSearchPaths_() \
list<std::string> LDSearchPaths( \
    "ld-search-path", \
    desc("Library search path"), \
    ZeroOrMore, \
    value_desc("directory"), \
    cat(MullCategory))

#define DebugCoverage_() \
  opt<bool> DebugCoverage( \
  "debug-coverage", \
  desc("Print coverage ranges"), \
  Optional, \
  init(false), \
  cat(MullCategory))

// clang-format on

namespace tool {

struct ReporterParameters {
  std::string reporterName;
  std::string reporterDirectory;
  std::string patchBasePathDir;
  bool compilationDatabaseAvailable;
  bool IDEReporterShowKilled;
  std::unordered_map<std::string, std::string> mullInformation;
};

class ReportersCLIOptions {
public:
  ReportersCLIOptions(mull::Diagnostics &diagnostics,
                      llvm::cl::list<mull::ReporterKind> &parameter);
  std::vector<std::unique_ptr<mull::Reporter>> reporters(ReporterParameters params);

private:
  mull::Diagnostics &diagnostics;
  llvm::cl::list<mull::ReporterKind> &parameter;
};

void dumpCLIInterface(mull::Diagnostics &diagnostics);
void dumpCLIInterface(mull::Diagnostics &diagnostics,
                      const std::vector<llvm::cl::Option *> &options, llvm::cl::Option *reporters);
void dumpMutators(mull::Diagnostics &diagnostics);

} // namespace tool
