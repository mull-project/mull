#include "mull/Filters/Filters.h"

#include "mull/Filters/BlockAddressFunctionFilter.h"
#include "mull/Filters/CoverageFilter.h"
#include "mull/Filters/FilePathFilter.h"
#include "mull/Filters/GitDiffFilter.h"
#include "mull/Filters/ManualFilter.h"
#include "mull/Filters/NoDebugInfoFilter.h"
#include "mull/Filters/VariadicFunctionFilter.h"

#include "rust/mull-core/core.rs.h"

#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace mull;
using namespace std::string_literals;

Filters::Filters(const MullConfig &configuration, const MullDiagnostics &diagnostics)
    : functionFilters(), mutationFilters(), instructionFilters(), configuration(configuration),
      diagnostics(diagnostics) {}

void Filters::enableNoDebugFilter() {
  auto *filter = new mull::NoDebugInfoFilter;

  mutationFilters.push_back(filter);
  functionFilters.push_back(filter);
  instructionFilters.push_back(filter);
  storage.emplace_back(filter);
}

void Filters::enableFilePathFilter() {
  if (configuration.include_paths.empty() && configuration.exclude_paths.empty()) {
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: both includePaths and excludePaths are empty");
    }
    return;
  }

  auto *filter = new mull::FilePathFilter;
  storage.emplace_back(filter);
  mutationFilters.push_back(filter);
  mutantFilters.push_back(filter);
  functionFilters.push_back(filter);

  for (const auto &regex : configuration.exclude_paths) {
    auto std_regex = std::string(regex);
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: excluding: "s + std_regex);
    }
    auto added = filter->exclude(std_regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for exclude-path: '" << std_regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
  for (const auto &regex : configuration.include_paths) {
    auto std_regex = std::string(regex);
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: including: "s + std_regex);
    }
    auto added = filter->include(std_regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for include-path: '" << std_regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
}

void Filters::enableGitDiffFilter() {
  auto projectRoot = std::string(configuration.git_project_root);
  if (!configuration.git_diff_ref.empty()) {
    if (projectRoot.empty()) {
      std::stringstream debugMessage;
      debugMessage
          << "gitDiffRef option has been provided but the path to the Git project root has not "
             "been specified via gitProjectRoot. The incremental testing will be disabled.";
      diagnostics.warning(debugMessage.str());
    } else if (!llvm::sys::fs::is_directory(projectRoot)) {
      std::stringstream debugMessage;
      debugMessage << "directory provided by gitProjectRoot does not exist, ";
      debugMessage << "the incremental testing will be disabled: ";
      debugMessage << projectRoot;
      diagnostics.warning(debugMessage.str());
    } else {
      llvm::SmallString<256> tmpGitProjectRoot;
      if (!llvm::sys::fs::real_path(projectRoot, tmpGitProjectRoot)) {
        projectRoot = tmpGitProjectRoot.str();

        std::string gitDiffBranch = std::string(configuration.git_diff_ref);
        diagnostics.info(std::string("Incremental testing using Git Diff is enabled.\n") +
                         "- Git ref: " + gitDiffBranch + "\n" +
                         "- Git project root: " + projectRoot);
        mull::GitDiffFilter *gitDiffFilter = mull::GitDiffFilter::createFromGitDiff(
            configuration, diagnostics, projectRoot, gitDiffBranch);

        if (gitDiffFilter) {
          storage.emplace_back(gitDiffFilter);
          instructionFilters.push_back(gitDiffFilter);
          mutantFilters.push_back(gitDiffFilter);
        }
      } else {
        diagnostics.warning(std::string("could not expand gitProjectRoot to an absolute path: ") +
                            projectRoot);
      }
    }
  }
}

CoverageFilter *Filters::enableCoverageFilter(const std::string &profileName,
                                              const std::vector<std::string> &objects) {
  auto filter = new CoverageFilter(configuration, diagnostics, profileName, objects);
  storage.emplace_back(filter);
  mutantFilters.push_back(filter);
  return filter;
}

void Filters::enableBlockAddressFilter() {
  auto filter = new mull::BlockAddressFunctionFilter;
  storage.emplace_back(filter);
  functionFilters.push_back(filter);
}

void Filters::enableVariadicFunctionFilter() {
  auto filter = new mull::VariadicFunctionFilter;
  storage.emplace_back(filter);
  functionFilters.push_back(filter);
}

void Filters::enableManualFilter() {
  auto filter = new mull::ManualFilter(diagnostics);
  storage.emplace_back(filter);
  mutationFilters.push_back(filter);
}
