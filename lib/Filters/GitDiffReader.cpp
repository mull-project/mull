#include "mull/Filters/GitDiffReader.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Runner.h"
#include <mull/Path.h>

#include <iostream>
#include <regex>
#include <sstream>

using namespace mull;

GitDiffReader::GitDiffReader(Diagnostics &diagnostics, const std::string gitRepoPath)
    : diagnostics(diagnostics), gitRepoPath(gitRepoPath) {}

GitDiffInfo GitDiffReader::readGitDiff(const std::string &gitBranch) {
  /// The implementation is borrowed from the git-clang-format Python tool.
  /// https://opensource.apple.com/source/clang/clang-800.0.38/src/tools/clang/tools/clang-format/git-clang-format.auto.html
  Runner runner(diagnostics);

  std::vector<std::string> args = { "diff", "-U0", gitBranch };

  ExecutionResult result = runner.runProgram("git", args, {}, 5000, true, false, gitRepoPath);
  if (result.exitStatus != 0) {
    diagnostics.warning(
        std::string("GitDiffReader: cannot get git diff information. Received output: ") +
        result.stderrOutput);
    return GitDiffInfo();
  }

  const std::string diffContent = result.stdoutOutput;
  GitDiffInfo gitDiffInfo = parseDiffContent(diffContent);
  return gitDiffInfo;
}

GitDiffInfo GitDiffReader::parseDiffContent(const std::string &diffContent) {
  GitDiffInfo gitDiffInfo;
  if (diffContent.empty()) {
    return gitDiffInfo;
  }

  std::stringstream diffStream(diffContent);
  std::string currentLine;

  std::string currentFileName;

  while (std::getline(diffStream, currentLine, '\n')) {
    std::regex lineRegex("^\\+\\+\\+\\ [^/]+/(.*)");
    std::regex rangeRegex("^@@ -[0-9,]+ \\+(\\d+)(,(\\d+))?");
    std::smatch matches;

    if (std::regex_search(currentLine, matches, lineRegex)) {
      currentFileName = absoluteFilePath(this->gitRepoPath, matches[1].str());
      continue;
    }

    if (std::regex_search(currentLine, matches, rangeRegex)) {
      const std::string startLineStr = matches[1].str();
      int startLine = std::stoi(startLineStr);

      size_t lineCount = 1;
      const std::string lineCountStr = matches[3].str();
      if (lineCountStr.size() > 0) {
        lineCount = std::stoi(lineCountStr);
      }
      if (lineCount > 0) {
        if (gitDiffInfo.count(currentFileName) == 0) {
          gitDiffInfo[currentFileName] = std::vector<GitDiffSourceFileRange>();
        }
        gitDiffInfo[currentFileName].push_back(std::pair<int, int>(startLine, lineCount));
      }
    }
  }
  return gitDiffInfo;
}
