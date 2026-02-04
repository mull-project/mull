#pragma once

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/Filter.h"
#include "mull/Filters/GitDiffReader.h"
#include "mull/Filters/InstructionFilter.h"
#include "mull/Filters/MutantFilter.h"

struct MullConfig;

namespace mull {
struct SourceLocation;

class GitDiffFilter : public InstructionFilter, public MutantFilter {
public:
  static GitDiffFilter *createFromGitDiff(const MullConfig &configuration, Diagnostics &diagnostics,
                                          const std::string &gitProjectRoot,
                                          const std::string &gitDiffBranch);

  GitDiffFilter(const MullConfig &configuration, Diagnostics &diagnostics, GitDiffInfo gitDiffInfo);

  std::string name() override;
  bool shouldSkip(llvm::Instruction *instruction) override;
  bool shouldSkip(Mutant *mutant) override;

private:
  bool shouldSkip(const SourceLocation &sourceLocation, const std::string &kind);

  const MullConfig &configuration;
  Diagnostics &diagnostics;
  const GitDiffInfo gitDiffInfo;
};
} // namespace mull
