#pragma once

#include <llvm/Support/CommandLine.h>
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Reporters/ASTSourceInfoProvider.h>
#include <mull/Reporters/Reporter.h>
#include <mull/Toolchain/Toolchain.h>

namespace tool {

using namespace mull;
using namespace llvm::cl;

extern OptionCategory MullCXXCategory;

extern opt<std::string> InputFile;

extern opt<unsigned> Workers;
extern opt<unsigned> Timeout;

extern opt<std::string> ReportName;
extern opt<std::string> ReportDirectory;

extern opt<bool> DisableJunkDetection;
extern opt<std::string> CompilationDatabasePath;
extern opt<std::string> CompilationFlags;

extern opt<std::string> Linker;
extern opt<std::string> LinkerFlags;

extern opt<std::string> CoverageInfo;

extern opt<bool> DryRunOption;

enum MutatorsOptionIndex : int { _mutatorsOptionIndex_unused };

extern list<MutatorsOptionIndex> Mutators;
extern list<ReporterKind> ReportersOption;
extern opt<bool> IDEReporterShowKilled;

extern opt<bool> DebugEnabled;
extern opt<bool> StrictModeEnabled;

extern opt<bool> NoOutput;
extern opt<bool> NoTestOutput;
extern opt<bool> NoMutantOutput;

extern opt<bool> EnableAST;

extern list<std::string> ExcludePaths;
extern list<std::string> IncludePaths;

class MutatorsCLIOptions {
public:
  MutatorsCLIOptions(Diagnostics &diagnostics, list<MutatorsOptionIndex> &parameter);
  std::vector<std::unique_ptr<mull::Mutator>> mutators();
  std::vector<std::pair<std::string, std::string>> &getOptions();

private:
  MutatorsFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  list<MutatorsOptionIndex> &parameter;
};

struct ReporterParameters {
  std::string reporterName;
  std::string reporterDirectory;
  SourceInfoProvider &sourceInfoProvider;
};

class ReportersCLIOptions {
public:
  ReportersCLIOptions(Diagnostics &diagnostics, list<ReporterKind> &parameter);
  std::vector<std::unique_ptr<Reporter>> reporters(ReporterParameters params);

private:
  Diagnostics &diagnostics;
  list<ReporterKind> &parameter;
};

extern opt<bool> DumpCLIInterface;
void dumpCLIInterface(Diagnostics &diagnostics);

extern opt<bool> DumpMutators;
void dumpMutators(Diagnostics &diagnostics);

} // namespace tool
