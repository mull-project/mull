#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

using namespace std::string_literals;

namespace mull {

int MullDefaultTimeoutMilliseconds = 3000;

Configuration::Configuration()
    : pathOnDisk(), debugEnabled(false), quiet(true), silent(false), dryRunEnabled(false),
      captureTestOutput(true), captureMutantOutput(true), includeNotCovered(false),
      junkDetectionDisabled(false), timeout(MullDefaultTimeoutMilliseconds),
      diagnostics(IDEDiagnosticsKind::None),
      parallelization(ParallelizationConfig::defaultConfig()) {}

static std::vector<std::string> toStdVector(const rust::Vec<rust::String> &v) {
  std::vector<std::string> out;
  out.reserve(v.size());
  for (const auto &s : v) {
    out.emplace_back(std::string(s));
  }
  return out;
}

void Configuration::populateFromRustConfig(const MullConfig &c, const std::string &configPath,
                                           Diagnostics &diag) {
  debugEnabled = c.debug_enabled;
  timeout = c.timeout;
  quiet = c.quiet;
  silent = c.silent;
  dryRunEnabled = c.dry_run_enabled;
  captureTestOutput = c.capture_test_output;
  captureMutantOutput = c.capture_mutant_output;
  includeNotCovered = c.include_not_covered;
  junkDetectionDisabled = c.junk_detection_disabled;
  compilationDatabasePath = std::string(c.compilation_database_path);
  gitDiffRef = std::string(c.git_diff_ref);
  gitProjectRoot = std::string(c.git_project_root);
  mutators = toStdVector(c.mutators);
  ignoreMutators = toStdVector(c.ignore_mutators);
  compilerFlags = toStdVector(c.compiler_flags);
  includePaths = toStdVector(c.include_paths);
  excludePaths = toStdVector(c.exclude_paths);

  debug.printIR = c.debug.print_ir;
  debug.printIRBefore = c.debug.print_ir_before;
  debug.printIRAfter = c.debug.print_ir_after;
  debug.printIRToFile = c.debug.print_ir_to_file;
  debug.traceMutants = c.debug.trace_mutants;
  debug.coverage = c.debug.coverage;
  debug.gitDiff = c.debug.git_diff;
  debug.filters = c.debug.filters;
  debug.slowIRVerification = c.debug.slow_ir_verification;

  if (c.workers > 0) {
    ParallelizationConfig parallelizationConfig;
    parallelizationConfig.workers = c.workers;
    parallelizationConfig.normalize();
    if (parallelizationConfig.exceedsHardware()) {
      diag.warning("You choose a number of workers that exceeds your number of cores. "
                   "This may lead to timeouts and incorrect results");
    }
    parallelization = parallelizationConfig;
  } else {
    parallelization = ParallelizationConfig::defaultConfig();
  }
  if (c.execution_workers > 0) {
    parallelization.executionWorkers = c.execution_workers;
  }

  if (!configPath.empty()) {
    pathOnDisk = configPath;
    diag.info("Using config "s + configPath);
  }
}

} // namespace mull
