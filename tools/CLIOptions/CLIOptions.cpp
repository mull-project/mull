#include "CLIOptions.h"
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Reporters/GithubAnnotationsReporter.h>
#include <mull/Reporters/IDEReporter.h>
#include <mull/Reporters/MutationTestingElementsReporter.h>
#include <mull/Reporters/PatchesReporter.h>
#include <mull/Reporters/SQLiteReporter.h>

#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace mull;
using namespace tool;
using namespace std::string_literals;

std::vector<std::string> tool::toStdVector(const rust::Vec<rust::String> &v) {
  std::vector<std::string> out;
  out.reserve(v.size());
  for (const auto &s : v) {
    out.emplace_back(std::string(s));
  }
  return out;
}

void tool::populateConfiguration(Configuration &configuration, const CliConfig &cli,
                                 Diagnostics &diagnostics) {
  const auto &c = cli.config;
  configuration.debugEnabled = c.debug_enabled;
  configuration.timeout = c.timeout;
  configuration.quiet = c.quiet;
  configuration.silent = c.silent;
  configuration.dryRunEnabled = c.dry_run_enabled;
  configuration.captureTestOutput = c.capture_test_output;
  configuration.captureMutantOutput = c.capture_mutant_output;
  configuration.includeNotCovered = c.include_not_covered;
  configuration.junkDetectionDisabled = c.junk_detection_disabled;
  configuration.compilationDatabasePath = std::string(c.compilation_database_path);
  configuration.gitDiffRef = std::string(c.git_diff_ref);
  configuration.gitProjectRoot = std::string(c.git_project_root);
  configuration.mutators = toStdVector(c.mutators);
  configuration.ignoreMutators = toStdVector(c.ignore_mutators);
  configuration.compilerFlags = toStdVector(c.compiler_flags);
  configuration.includePaths = toStdVector(c.include_paths);
  configuration.excludePaths = toStdVector(c.exclude_paths);

  configuration.debug.printIR = c.debug.print_ir;
  configuration.debug.printIRBefore = c.debug.print_ir_before;
  configuration.debug.printIRAfter = c.debug.print_ir_after;
  configuration.debug.printIRToFile = c.debug.print_ir_to_file;
  configuration.debug.traceMutants = c.debug.trace_mutants;
  configuration.debug.coverage = c.debug.coverage;
  configuration.debug.gitDiff = c.debug.git_diff;
  configuration.debug.filters = c.debug.filters;
  configuration.debug.slowIRVerification = c.debug.slow_ir_verification;

  if (c.workers > 0) {
    ParallelizationConfig parallelizationConfig;
    parallelizationConfig.workers = c.workers;
    parallelizationConfig.normalize();
    if (parallelizationConfig.exceedsHardware()) {
      diagnostics.warning("You choose a number of workers that exceeds your number of cores. "
                          "This may lead to timeouts and incorrect results");
    }
    configuration.parallelization = parallelizationConfig;
  } else {
    configuration.parallelization = ParallelizationConfig::defaultConfig();
  }
  if (c.execution_workers > 0) {
    configuration.parallelization.executionWorkers = c.execution_workers;
  }

  if (!std::string(cli.config_path).empty()) {
    configuration.pathOnDisk = std::string(cli.config_path);
    diagnostics.info("Using config "s + std::string(cli.config_path));
  }
}

void tool::setupDiagnostics(Diagnostics &diagnostics, const CliConfig &cli) {
  if (cli.config.debug_enabled) {
    diagnostics.enableDebugMode();
    diagnostics.debug("Diagnostics: Debug Mode enabled. Debug-level messages will be printed.");
  }
  if (cli.strict) {
    diagnostics.enableStrictMode();
    diagnostics.info("Diagnostics: Strict Mode enabled. Warning messages will be treated as "
                     "fatal errors.");
  }
}

ReporterParameters
tool::buildReporterParameters(const CliConfig &cli,
                              std::unordered_map<std::string, std::string> mullInformation) {
  return ReporterParameters{
    .reporters = toStdVector(cli.reporters),
    .reporterName = std::string(cli.report_name),
    .reporterDirectory = std::string(cli.report_dir),
    .patchBasePathDir = std::string(cli.report_patch_base),
    .compilationDatabaseAvailable = true,
    .IDEReporterShowKilled = cli.ide_reporter_show_killed,
    .mullInformation = std::move(mullInformation),
  };
}

std::vector<std::unique_ptr<Reporter>> tool::createReporters(Diagnostics &diagnostics,
                                                             const ReporterParameters &params) {
  std::vector<std::unique_ptr<Reporter>> reporters;

  for (const auto &r : params.reporters) {
    if (r == "IDE") {
      reporters.emplace_back(new IDEReporter(diagnostics,
                                             params.IDEReporterShowKilled,
                                             params.reporterDirectory,
                                             params.reporterName));
    } else if (r == "SQLite") {
      reporters.emplace_back(new SQLiteReporter(
          diagnostics, params.reporterDirectory, params.reporterName, params.mullInformation));
    } else if (r == "Elements") {
      if (!params.compilationDatabaseAvailable) {
        diagnostics.warning("Mutation Testing Elements Reporter may not work without "
                            "compilation database. Consider providing -compdb-path or "
                            "-compilation-flags.");
      }
      reporters.emplace_back(new MutationTestingElementsReporter(
          diagnostics, params.reporterDirectory, params.reporterName, params.mullInformation));
    } else if (r == "Patches") {
      reporters.emplace_back(new PatchesReporter(diagnostics,
                                                 params.reporterDirectory,
                                                 params.reporterName,
                                                 params.patchBasePathDir,
                                                 params.mullInformation));
    } else if (r == "GithubAnnotations") {
      reporters.emplace_back(new GithubAnnotationsReporter(diagnostics));
    } else {
      diagnostics.warning("Unknown reporter: " + r);
    }
  }

  if (reporters.empty()) {
    reporters.emplace_back(new IDEReporter(
        diagnostics, params.IDEReporterShowKilled, params.reporterDirectory, params.reporterName));
  }

  return reporters;
}

rust::Vec<rust::String> tool::argsFromArgv(int argc, char **argv) {
  rust::Vec<rust::String> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(rust::String(argv[i]));
  }
  return args;
}

void tool::dumpMutators(Diagnostics &diagnostics, std::string out) {
  MutatorsFactory factory(diagnostics);
  factory.init();

  std::vector<Mutator *> availableMutators;
  for (const auto &pair : factory.getMutatorsMapping()) {
    availableMutators.push_back(pair.second.get());
  }

  std::sort(availableMutators.begin(), availableMutators.end(), [&](Mutator *lhs, Mutator *rhs) {
    return lhs->getUniqueIdentifier() < rhs->getUniqueIdentifier();
  });

  auto sanitizeString = [](const std::string &input) -> std::string {
    std::string result = input;
    auto replaceOne = [&](const std::string &replacement) {
      std::string escape("\\");
      auto pos = result.find(replacement);
      if (pos != std::string::npos) {
        result.replace(pos, replacement.size(), escape + replacement);
      }
    };
    replaceOne("|=");
    replaceOne("*=");
    return result;
  };

  std::stringstream replacements;
  std::stringstream table;

  std::string operatorHeaderName("Operator Name");
  std::string operatorHeader(operatorHeaderName.size(), '=');
  std::string semanticsHeaderName("Operator Semantics");
  std::string semanticsHeader(semanticsHeaderName.size(), '=');

  table << operatorHeader << " " << semanticsHeader << "\n";
  table << operatorHeaderName << " " << semanticsHeaderName << "\n";
  table << operatorHeader << " " << semanticsHeader << "\n";

  for (size_t i = 0; i < availableMutators.size(); i++) {
    Mutator *mutator = availableMutators[i];
    std::string n = std::to_string(i);
    std::string name("op" + n);
    std::string description("desc" + n);

    std::string padding(operatorHeader.size() - name.size() - 1, ' ');
    table << "|" << name << "|" << padding << "|" << description << "|\n";

    replacements << ".. |" << name << "| replace:: " << mutator->getUniqueIdentifier() << "\n";
    replacements << ".. |" << description
                 << "| replace:: " << sanitizeString(mutator->getDescription()) << "\n";
  }

  replacements << "\n\n";
  table << operatorHeader << " " << semanticsHeader << "\n";

  table << "\n\n";
  table << "Groups:\n";
  for (const auto &pair : factory.getGroupsMapping()) {
    table << "    :" << pair.first << ":\t" << MutatorsFactory::descriptionForGroup(pair.second)
          << "\n\n";
  }

  std::error_code ec;
  llvm::raw_fd_ostream os(out, ec);
  if (ec) {
    diagnostics.error(ec.message());
  }
  os << replacements.str();
  auto s = table.str();
  // trim trailing newline
  os << s.substr(0, s.size() - 1);
}
