#include "mull/Filters/Filters.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/BlockAddressFunctionFilter.h"
#include "mull/Filters/CoverageFilter.h"
#include "mull/Filters/FilePathFilter.h"
#include "mull/Filters/GitDiffFilter.h"
#include "mull/Filters/NoDebugInfoFilter.h"
#include "mull/Filters/VariadicFunctionFilter.h"
#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace mull;
using namespace std::string_literals;

Filters::Filters(const Configuration &configuration, Diagnostics &diagnostics)
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
  if (configuration.includePaths.empty() && configuration.excludePaths.empty()) {
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

  for (const auto &regex : configuration.excludePaths) {
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: excluding: "s + regex);
    }
    auto added = filter->exclude(regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for exclude-path: '" << regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
  for (const auto &regex : configuration.includePaths) {
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: including: "s + regex);
    }
    auto added = filter->include(regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for include-path: '" << regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
}

void Filters::enableGitDiffFilter() {
  if (!configuration.gitDiffRef.empty()) {
    if (configuration.gitProjectRoot.empty()) {
      std::stringstream debugMessage;
      debugMessage
          << "-git-diff-ref option has been provided but the path to the Git project root has not "
             "been specified via -git-project-root. The incremental testing will be disabled.";
      diagnostics.warning(debugMessage.str());
    } else if (!llvm::sys::fs::is_directory(configuration.gitProjectRoot)) {
      std::stringstream debugMessage;
      debugMessage << "directory provided by -git-project-root does not exist, ";
      debugMessage << "the incremental testing will be disabled: ";
      debugMessage << configuration.gitProjectRoot;
      diagnostics.warning(debugMessage.str());
    } else {
      std::string gitProjectRoot = configuration.gitProjectRoot;
      llvm::SmallString<256> tmpGitProjectRoot;
      if (!llvm::sys::fs::real_path(gitProjectRoot, tmpGitProjectRoot)) {
        gitProjectRoot = tmpGitProjectRoot.str();

        std::string gitDiffBranch = configuration.gitDiffRef;
        diagnostics.info(std::string("Incremental testing using Git Diff is enabled.\n") +
                         "- Git ref: " + gitDiffBranch + "\n" +
                         "- Git project root: " + gitProjectRoot);
        mull::GitDiffFilter *gitDiffFilter = mull::GitDiffFilter::createFromGitDiff(
            configuration, diagnostics, gitProjectRoot, gitDiffBranch);

        if (gitDiffFilter) {
          storage.emplace_back(gitDiffFilter);
          instructionFilters.push_back(gitDiffFilter);
          mutantFilters.push_back(gitDiffFilter);
        }
      } else {
        diagnostics.warning(
            std::string("could not expand -git-project-root to an absolute path: ") +
            gitProjectRoot);
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
