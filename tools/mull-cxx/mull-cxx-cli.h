#pragma once

#include "CLIOptions.h"

namespace tool {
using namespace mull;
using namespace llvm::cl;

OptionCategory MullCategory("mull-cxx");
InputFile_();
OutputFile_();
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
Mutators_();
DryRunOption_();
Linker_();
LinkerFlags_();
LinkerTimeout_();
CoverageInfo_();
IncludeNotCovered_();
KeepExecutable_();
KeepObjectFiles_();
CompilationDatabasePath_();
CompilationFlags_();
ExcludePaths_();
IncludePaths_();
GitDiffRef_();
GitProjectRoot_();
EnableAST_();
DisableJunkDetection_();
IDEReporterShowKilled_();
MutateOnly_();

void dumpCLIInterface(Diagnostics &diagnostics) {
  // Enumerating CLI options explicitly to control the order and what to show
  Option *mutators = &(Option &)Mutators;
  Option *reporters = &(Option &)ReportersOption;
  std::vector<Option *> mullOptions({
      &OutputFile,

      &Workers,
      &Timeout,
      &DryRunOption,
      &MutateOnly,

      &ReportName,
      &ReportDirectory,
      &EnableAST,
      reporters,
      &IDEReporterShowKilled,
      &DebugEnabled,
      &StrictModeEnabled,

      &KeepObjectFiles,
      &KeepExecutable,

      &NoTestOutput,
      &NoMutantOutput,
      &NoOutput,

      &DisableJunkDetection,
      &CompilationDatabasePath,
      &CompilationFlags,

      &Linker,
      &LinkerFlags,
      &LinkerTimeout,

      &CoverageInfo,
      &IncludeNotCovered,

      &(Option &)IncludePaths,
      &(Option &)ExcludePaths,

      &(Option &)GitDiffRef,
      &(Option &)GitProjectRoot,

      mutators,
  });
  dumpCLIInterface(diagnostics, mullOptions, reporters, mutators);
}

} // namespace tool
