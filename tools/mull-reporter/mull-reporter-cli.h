#pragma once

#include "tools/CLIOptions/CLIOptions.h"

namespace tool {
using namespace mull;
using namespace llvm::cl;

OptionCategory MullCategory("mull-reporter");
SQLiteReport_();
DumpCLIInterface_();
ReportersOption_();
DebugEnabled_();
StrictModeEnabled_();
AllowSurvivingEnabled_();
MutationScoreThreshold_();
NoOutput_();
NoTestOutput_();
NoMutantOutput_();
ReportName_();
ReportDirectory_();
ReportPatchBaseDirectory_();
IDEReporterShowKilled_();

void dumpCLIInterface(mull::Diagnostics &diagnostics, std::string out) {
  // Enumerating CLI options explicitly to control the order and what to show
  Option *reporters = &(Option &)ReportersOption;
  std::vector<Option *> mullOptions({
      &(Option &)SQLiteReport,

      &ReportName,
      &ReportDirectory,
      &ReportPatchBaseDirectory,
      reporters,
      &IDEReporterShowKilled,
      &AllowSurvivingEnabled,
      &MutationScoreThreshold,

      &NoTestOutput,
      &NoMutantOutput,
      &NoOutput,
      &DebugEnabled,
      &StrictModeEnabled,
  });
  dumpCLIInterface(diagnostics, mullOptions, reporters, out);
}

} // namespace tool
