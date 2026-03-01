#include "CLIOptions.h"
#include <mull/Reporters/GithubAnnotationsReporter.h>
#include <mull/Reporters/IDEReporter.h>
#include <mull/Reporters/MutationTestingElementsReporter.h>
#include <mull/Reporters/PatchesReporter.h>
#include <mull/Reporters/SQLiteReporter.h>

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

std::vector<std::unique_ptr<Reporter>> tool::createReporters(const MullDiagnostics &diagnostics,
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
    } else if (r == "GithubAnnotations" || r == "GitHubAnnotations") {
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
