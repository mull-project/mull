#pragma once

#include <llvm/Support/CommandLine.h>
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
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

extern list<std::string> LDSearchPaths;
extern list<std::string> ExcludePaths;
extern list<std::string> IncludePaths;

enum SandboxType { None, Watchdog, Timer };
enum ReporterType { IDE, SQLite, Elements };

extern opt<SandboxType> SandboxOption;
extern list<ReporterType> ReporterOption;

class MutatorsCLIOptions {
public:
  explicit MutatorsCLIOptions(list<MutatorsOptionIndex> &parameter);
  std::vector<std::unique_ptr<mull::Mutator>> mutators();

private:
  MutatorsFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  list<MutatorsOptionIndex> &parameter;
};

class TestFrameworkCLIOptions {
public:
  explicit TestFrameworkCLIOptions(opt<TestFrameworkOptionIndex> &parameter);
  TestFramework testFramework(Toolchain &toolchain, Configuration &configuration);

private:
  TestFrameworkFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  opt<TestFrameworkOptionIndex> &parameter;
};

} // namespace tool