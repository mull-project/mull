#pragma once

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/Filter.h"
#include "mull/Filters/GitDiffReader.h"
#include "mull/Filters/InstructionFilter.h"

namespace mull {
struct SourceLocation;
class GitDiffFilter : public InstructionFilter {
public:
  static GitDiffFilter *createFromGitDiff(Diagnostics &diagnostics,
                                          const std::string &gitProjectRoot,
                                          const std::string &gitDiffBranch);

  GitDiffFilter(Diagnostics &diagnostics, GitDiffInfo gitDiffInfo);

  std::string name() override;
  bool shouldSkip(llvm::Instruction *instruction) const override;

private:
  Diagnostics &diagnostics;
  const GitDiffInfo gitDiffInfo;
};
} // namespace mull
