#pragma once

#include "CLIOptions.h"

namespace tool {
using namespace mull;
using namespace llvm::cl;

OptionCategory MullCategory("mull-runner");
InputFile_();
DumpCLIInterface_();
ReportersOption_();
DebugEnabled_();
StrictModeEnabled_();
Timeout_();
Workers_();
NoOutput_();
NoTestOutput_();
NoMutantOutput_();
ReportName_();
ReportDirectory_();
IDEReporterShowKilled_();
IncludeNotCovered_();
RunnerArgs_();
TestProgram_();

void dumpCLIInterface(mull::Diagnostics &diagnostics) {
  // Enumerating CLI options explicitly to control the order and what to show
  Option *reporters = &(Option &)ReportersOption;
  std::vector<Option *> mullOptions({
      &(Option &)TestProgram,

      &Workers,
      &Timeout,

      &ReportName,
      &ReportDirectory,
      reporters,
      &IDEReporterShowKilled,
      &DebugEnabled,
      &StrictModeEnabled,

      &NoTestOutput,
      &NoMutantOutput,
      &NoOutput,
  });
  dumpCLIInterface(diagnostics, mullOptions, reporters, nullptr);
}

} // namespace tool
