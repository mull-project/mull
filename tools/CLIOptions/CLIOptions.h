#pragma once

#include <llvm/Support/CommandLine.h>
#include <mull/Config/Configuration.h>
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Reporters/ASTSourceInfoProvider.h>
#include <mull/Reporters/Reporter.h>
#include <mull/Toolchain/Toolchain.h>
#include <sstream>

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

#define OutputFile_() \
opt<std::string> OutputFile( \
    "output", \
    desc("output file"), \
    value_desc("path"), \
    init(""), \
    cat(MullCategory))\

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

#define DisableJunkDetection_() \
opt<bool> DisableJunkDetection( \
    "disable-junk-detection", \
    desc("Do not remove junk mutations"), \
    Optional, \
    init(false), \
    cat(MullCategory))

#define CompilationDatabasePath_() \
opt<std::string> CompilationDatabasePath( \
    "compdb-path", \
    desc("Path to a compilation database (compile_commands.json) for junk detection"), \
    value_desc("filename"), \
    Optional, \
    cat(MullCategory))

#define CompilationFlags_() \
opt<std::string> CompilationFlags( \
    "compilation-flags", \
    desc("Extra compilation flags for junk detection"), \
    value_desc("string"), \
    Optional, \
    cat(MullCategory))

#define Linker_() \
opt<std::string> Linker( \
    "linker", \
    desc("Linker program"), \
    value_desc("string"), \
    Optional, \
    init("clang"), \
    cat(MullCategory))

#define LinkerFlags_() \
opt<std::string> LinkerFlags( \
    "linker-flags", \
    desc("Extra linker flags to produce final executable"), \
    value_desc("string"), \
    Optional, \
    init(std::string()), \
    cat(MullCategory))

#define LinkerTimeout_() \
opt<unsigned> LinkerTimeout( \
    "linker-timeout", \
    desc("Timeout for the linking job (milliseconds)"), \
    Optional, \
    value_desc("number"), \
    init(MullDefaultLinkerTimeoutMilliseconds), \
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

#define MutateOnly_() \
opt<bool> MutateOnly( \
    "mutate-only", \
    desc("Skips mutant execution. Unlike -dry-run generates mutants. Disabled by default"), \
    Optional, \
    init(false), \
    cat(MullCategory))

#define Mutators_() \
list<MutatorsOptionIndex> Mutators( \
    "mutators", \
    desc("Choose mutators:"), \
    ZeroOrMore, \
    value_desc("mutator"), \
    cat(MullCategory))

#define ExcludePaths_() \
list<std::string> ExcludePaths( \
    "exclude-path", \
    desc("File/directory paths to ignore (supports regex, equivalent to \"grep -E\")"), \
    ZeroOrMore, \
    value_desc("regex"), \
    cat(MullCategory))

#define IncludePaths_() \
list<std::string> IncludePaths( \
    "include-path", \
    desc("File/directory paths to whitelist (supports regex, equivalent to \"grep -E\")"), \
    ZeroOrMore, \
    value_desc("regex"), \
    cat(MullCategory))

#define EnableAST_() \
opt<bool> EnableAST( \
    "enable-ast", llvm::cl::Optional, \
    llvm::cl::desc("Enable \"white\" AST search (disabled by default)"), \
    llvm::cl::cat(MullCategory), llvm::cl::init(false))

#define KeepObjectFiles_() \
opt<bool> KeepObjectFiles( \
    "keep-object-files", llvm::cl::Optional, \
    llvm::cl::desc("Keep temporary object files"), \
    llvm::cl::cat(MullCategory), llvm::cl::init(false))

#define KeepExecutable_() \
opt<bool> KeepExecutable( \
    "keep-executable", llvm::cl::Optional, \
    llvm::cl::desc("Keep temporary executable file"), \
    llvm::cl::cat(MullCategory), llvm::cl::init(false))

#define GitDiffRef_() \
opt<std::string> GitDiffRef( \
    "git-diff-ref", \
    desc("Git branch, commit, or tag to run diff against (enables incremental testing)"), \
    Optional, \
    value_desc("ref"), \
    cat(MullCategory))

#define GitProjectRoot_() \
opt<std::string> GitProjectRoot( \
    "git-project-root", \
    desc("Path to project's Git root (used together with -git-diff-ref)"), \
    Optional, \
    value_desc("path"), \
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

// clang-format on

namespace tool {

enum MutatorsOptionIndex : int { _mutatorsOptionIndex_unused };

class MutatorsCLIOptions {
public:
  MutatorsCLIOptions(mull::Diagnostics &diagnostics,
                     llvm::cl::list<MutatorsOptionIndex> &parameter);
  std::vector<std::unique_ptr<mull::Mutator>> mutators();
  std::vector<std::pair<std::string, std::string>> &getOptions();

private:
  mull::MutatorsFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  llvm::cl::list<MutatorsOptionIndex> &parameter;
};

struct ReporterParameters {
  std::string reporterName;
  std::string reporterDirectory;
  mull::SourceInfoProvider &sourceInfoProvider;
  bool compilationDatabaseAvailable;
  bool IDEReporterShowKilled;
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
                      const std::vector<llvm::cl::Option *> &options, llvm::cl::Option *reporters,
                      llvm::cl::Option *mutators);
void dumpMutators(mull::Diagnostics &diagnostics);

} // namespace tool
