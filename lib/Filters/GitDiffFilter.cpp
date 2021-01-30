#include "mull/Filters/GitDiffFilter.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/SourceLocation.h"
#include "mull/Toolchain/Runner.h"

#include "llvm/IR/InstIterator.h"
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/FileSystem.h>

#include <iostream>
#include <regex>
#include <sstream>
#include <unistd.h>

using namespace mull;

GitDiffFilter *GitDiffFilter::createFromGitDiff(Diagnostics &diagnostics,
                                                const std::string &gitProjectRoot,
                                                const std::string &gitDiffBranch) {
  mull::GitDiffReader gitDiffReader(diagnostics, gitProjectRoot);
  mull::GitDiffInfo gitDiffInfo = gitDiffReader.readGitDiff(gitDiffBranch);
  return new GitDiffFilter(diagnostics, gitDiffInfo);
}

GitDiffFilter::GitDiffFilter(Diagnostics &diagnostics, GitDiffInfo gitDiffInfo)
    : diagnostics(diagnostics), gitDiffInfo(gitDiffInfo) {}

std::string GitDiffFilter::name() {
  return "Git Diff";
}

bool GitDiffFilter::shouldSkip(llvm::Instruction *instruction) const {
  SourceLocation sourceLocation = SourceLocation::locationFromInstruction(instruction);
  if (sourceLocation.isNull()) {
    return true;
  }

  /// If no diff, then filtering out.
  if (gitDiffInfo.size() == 0) {
    std::stringstream debugMessage;
    debugMessage << "GitDiffFilter: git diff is empty. Skipping instruction: ";
    debugMessage << sourceLocation.filePath << ":";
    debugMessage << sourceLocation.line << ":" << sourceLocation.column;
    diagnostics.debug(debugMessage.str());
    return true;
  }

  /// If file is not in the diff, then filtering out.
  if (gitDiffInfo.count(sourceLocation.filePath) == 0) {
    std::stringstream debugMessage;
    debugMessage << "GitDiffFilter: the file is not present in the git diff. ";
    debugMessage << "Skipping instruction: ";
    debugMessage << sourceLocation.filePath << ":";
    debugMessage << sourceLocation.line << ":" << sourceLocation.column;
    diagnostics.debug(debugMessage.str());
    return true;
  }

  GitDiffSourceFileRanges ranges = gitDiffInfo.at(sourceLocation.filePath);
  for (auto &range : ranges) {
    int rangeEnd = range.first + range.second - 1;
    if (range.first <= sourceLocation.line && sourceLocation.line <= rangeEnd) {
      std::stringstream debugMessage;
      debugMessage << "GitDiffFilter: whitelisting instruction: ";
      debugMessage << sourceLocation.filePath << ":";
      debugMessage << sourceLocation.line << ":" << sourceLocation.column;
      diagnostics.debug(debugMessage.str());
      return false;
    }
  }

  std::stringstream debugMessage;
  debugMessage << "GitDiffFilter: skipping instruction: ";
  debugMessage << sourceLocation.filePath << ":";
  debugMessage << sourceLocation.line << ":" << sourceLocation.column;
  diagnostics.debug(debugMessage.str());

  return true;
}
