#include "WeakTestsReporter.h"

#include <llvm/Support/CommandLine.h>

using namespace llvm;

cl::OptionCategory MullOptionCategory("mull-reporter");

static cl::SubCommand WeakTestsCommand("weak-tests", "show weak tests");

static cl::opt<std::string> SqliteReportFile("sqlite-report",
                                             cl::desc("Path to sqlite file"),
                                             cl::cat(MullOptionCategory),
                                             cl::Required,
                                             cl::sub(WeakTestsCommand));

static cl::opt<int> LowerBound("lower-bound",
                               cl::desc(
                                 "Lower bound for mutation score (0-100)"),
                               cl::value_desc("0-100"),
                               cl::cat(MullOptionCategory),
                               cl::sub(WeakTestsCommand),
                               cl::init(100),
                               cl::Optional);

static cl::opt<bool> IncludeMutants("include-mutants",
                                    cl::desc("Also show mutants"),
                                    cl::cat(MullOptionCategory),
                                    cl::sub(WeakTestsCommand),
                                    cl::init(false),
                                    cl::Optional);

int main(int argc, char *argv[]) {
  cl::HideUnrelatedOptions(MullOptionCategory);
  cl::ParseCommandLineOptions(argc, argv, "mull-reporter");

  if (WeakTestsCommand) {
    mull::WeakTestsReporter reporter;
    reporter.showReport(SqliteReportFile.c_str(), LowerBound, IncludeMutants);
    return 0;
  }

  return 0;
}
