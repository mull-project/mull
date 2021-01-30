#pragma once

#include "mull/Diagnostics/Diagnostics.h"

#include <map>
#include <string>
#include <vector>

namespace mull {
class Diagnostics;

typedef std::pair<int, int> GitDiffSourceFileRange;
typedef std::vector<GitDiffSourceFileRange> GitDiffSourceFileRanges;
typedef std::map<std::string, GitDiffSourceFileRanges> GitDiffInfo;

class GitDiffReader {
public:
  GitDiffReader(Diagnostics &diagnostics, const std::string gitRepoPath);
  GitDiffInfo readGitDiff(const std::string &gitBranch);
  GitDiffInfo parseDiffContent(const std::string &diffContent);

private:
  Diagnostics &diagnostics;
  const std::string gitRepoPath;
};
} // namespace mull
