#include "mull/Filters/GitDiffFilter.h"

#include "mull/Mutant.h"
#include "mull/Runner.h"
#include "mull/SourceLocation.h"

#include "rust/mull-core/core.rs.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/FileSystem.h>

#include <sstream>
#include <utility>

using namespace mull;

GitDiffFilter *GitDiffFilter::createFromGitDiff(const MullConfig &configuration,
                                                const MullDiagnostics &diagnostics,
                                                const std::string &gitProjectRoot,
                                                const std::string &gitDiffBranch) {
  mull::GitDiffReader gitDiffReader(diagnostics, gitProjectRoot);
  mull::GitDiffInfo gitDiffInfo = gitDiffReader.readGitDiff(gitDiffBranch);
  return new GitDiffFilter(configuration, diagnostics, gitDiffInfo);
}

GitDiffFilter::GitDiffFilter(const MullConfig &configuration, const MullDiagnostics &diagnostics,
                             GitDiffInfo gitDiffInfo)
    : configuration(configuration), diagnostics(diagnostics), gitDiffInfo(std::move(gitDiffInfo)) {}

std::string GitDiffFilter::name() {
  return "Git Diff";
}

bool GitDiffFilter::shouldSkip(llvm::Instruction *instruction) {
  return shouldSkip(SourceLocation::locationFromInstruction(instruction), "instruction");
}

bool GitDiffFilter::shouldSkip(Mutant *mutant) {
  return shouldSkip(mutant->getSourceLocation(), "mutant");
}

bool GitDiffFilter::shouldSkip(const SourceLocation &sourceLocation, const std::string &kind) {
  if (sourceLocation.isNull()) {
    return true;
  }

  /// If no diff, then filtering out.
  if (gitDiffInfo.empty()) {
    if (configuration.debug.git_diff) {
      std::stringstream debugMessage;
      debugMessage << "GitDiffFilter: git diff is empty. Skipping " << kind << ": ";
      debugMessage << sourceLocation.filePath << ":";
      debugMessage << sourceLocation.line << ":" << sourceLocation.column;
      diagnostics.debug(debugMessage.str());
    }
    return true;
  }

  /// If file is not in the diff, then filtering out.
  if (gitDiffInfo.count(sourceLocation.filePath) == 0) {
    if (configuration.debug.git_diff) {
      std::stringstream debugMessage;
      debugMessage << "GitDiffFilter: the file is not present in the git diff. ";
      debugMessage << "Skipping " << kind << ": ";
      debugMessage << sourceLocation.filePath << ":";
      debugMessage << sourceLocation.line << ":" << sourceLocation.column;
      diagnostics.debug(debugMessage.str());
    }
    return true;
  }

  GitDiffSourceFileRanges ranges = gitDiffInfo.at(sourceLocation.filePath);
  for (auto &range : ranges) {
    unsigned rangeEnd = range.first + range.second - 1;
    if (range.first <= sourceLocation.line && sourceLocation.line <= rangeEnd) {
      if (configuration.debug.git_diff) {
        std::stringstream debugMessage;
        debugMessage << "GitDiffFilter: allowing " << kind << ": ";
        debugMessage << sourceLocation.filePath << ":";
        debugMessage << sourceLocation.line << ":" << sourceLocation.column;
        diagnostics.debug(debugMessage.str());
      }
      return false;
    }
  }

  if (configuration.debug.git_diff) {
    std::stringstream debugMessage;
    debugMessage << "GitDiffFilter: skipping " << kind << ": ";
    debugMessage << sourceLocation.filePath << ":";
    debugMessage << sourceLocation.line << ":" << sourceLocation.column;
    diagnostics.debug(debugMessage.str());
  }

  return true;
}
