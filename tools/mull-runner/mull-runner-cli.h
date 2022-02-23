#pragma once

#include "CLIOptions.h"

namespace tool {
using namespace mull;
using namespace llvm::cl;

OptionCategory MullCategory("mull-runner");
InputFile_();
DumpCLIInterface_();
DumpMutators_();
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
ReportPatchBaseDirectory_();
IDEReporterShowKilled_();
IncludeNotCovered_();
RunnerArgs_();
TestProgram_();
LDSearchPaths_();
CoverageInfo_();
DebugCoverage_();

void dumpCLIInterface(mull::Diagnostics &diagnostics) {
  // Enumerating CLI options explicitly to control the order and what to show
  Option *reporters = &(Option &)ReportersOption;
  std::vector<Option *> mullOptions({
      &(Option &)TestProgram,

      &Workers,
      &Timeout,

      &ReportName,
      &ReportDirectory,
      &ReportPatchBaseDirectory,
      reporters,
      &IDEReporterShowKilled,
      &DebugEnabled,
      &StrictModeEnabled,

      &NoTestOutput,
      &NoMutantOutput,
      &NoOutput,

      &(Option &)LDSearchPaths,
      &CoverageInfo,
      &DebugCoverage,
  });
  dumpCLIInterface(diagnostics, mullOptions, reporters);
}

} // namespace tool
