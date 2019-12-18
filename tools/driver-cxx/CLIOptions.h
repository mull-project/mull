#pragma once

#include <llvm/Support/CommandLine.h>
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Reporters/ASTSourceInfoProvider.h>
#include <mull/Reporters/Reporter.h>
#include <mull/Sandbox/ProcessSandbox.h>
#include <mull/TestFrameworks/TestFramework.h>
#include <mull/TestFrameworks/TestFrameworkFactory.h>
#include <mull/Toolchain/Toolchain.h>

namespace tool {

using namespace mull;
using namespace llvm::cl;

extern OptionCategory MullCXXCategory;

extern opt<std::string> InputFile;

extern opt<unsigned> Workers;

extern opt<std::string> ReportName;
extern opt<std::string> ReportDirectory;

extern opt<std::string> CompilationDatabasePath;
extern opt<std::string> CompilationFlags;

extern opt<bool> DryRunOption;
extern opt<bool> DisableCache;
extern opt<std::string> CacheDir;

enum MutatorsOptionIndex : int { _mutatorsOptionIndex_unused };
enum TestFrameworkOptionIndex : int { _testFrameworkOptionIndex_unused };

extern list<MutatorsOptionIndex> Mutators;
extern opt<TestFrameworkOptionIndex> TestFrameworks;
extern list<ReporterKind> ReportersOption;
extern opt<bool> IDEReporterShowKilled;

extern opt<SandboxKind> SandboxOption;

extern list<std::string> LDSearchPaths;
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

class TestFrameworkCLIOptions {
public:
  explicit TestFrameworkCLIOptions(Diagnostics &diagnostics,
                                   opt<TestFrameworkOptionIndex> &parameter);
  TestFramework testFramework(Toolchain &toolchain, Configuration &configuration);

private:
  Diagnostics &diagnostics;
  TestFrameworkFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  opt<TestFrameworkOptionIndex> &parameter;
};

class SandboxCLIOptions {
public:
  explicit SandboxCLIOptions(opt<SandboxKind> &parameter);
  std::unique_ptr<mull::ProcessSandbox> sandbox();

private:
  opt<SandboxKind> &parameter;
};

extern opt<bool> DumpCLIInterface;
void dumpCLIInterface(Diagnostics &diagnostics);

extern opt<bool> DumpMutators;
void dumpMutators(Diagnostics &diagnostics);

} // namespace tool