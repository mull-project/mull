#pragma once

#include <map>
#include <string>
#include <vector>

struct MullDiagnostics;

namespace mull {

typedef std::pair<unsigned, unsigned> GitDiffSourceFileRange;
typedef std::vector<GitDiffSourceFileRange> GitDiffSourceFileRanges;
typedef std::map<std::string, GitDiffSourceFileRanges> GitDiffInfo;

class GitDiffReader {
public:
  GitDiffReader(const MullDiagnostics &diagnostics, const std::string gitRepoPath);
  GitDiffInfo readGitDiff(const std::string &gitBranch);
  GitDiffInfo parseDiffContent(const std::string &diffContent);

private:
  const MullDiagnostics &diagnostics;
  const std::string gitRepoPath;
};
} // namespace mull
