#include "mull/Reporters/SourceCodeReader.h"

#include "mull/SourceLocation.h"

#include <llvm/Support/FileSystem.h>

#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>

using namespace mull;

SourceCodeReader::SourceCodeReader() : sourceManager() {}

std::string SourceCodeReader::getContext(const mull::SourceLocation &sourceLocation) {
  std::stringstream ss;

  if (sourceLocation.isNull() || sourceLocation.line == 0 || sourceLocation.column == 0 ||
      sourceLocation.filePath.empty()) {
    ss << "Source code information is unavailable. Possibly a junk mutation.";
    return ss.str();
  }

  if (!llvm::sys::fs::exists(sourceLocation.filePath)) {
    ss << "File does not exist: " << sourceLocation.filePath;
    return ss.str();
  }

  const std::string delimiter = ":";

  const size_t totalLines = sourceManager.getNumberOfLines(sourceLocation);

  const size_t maxLineNumber = std::min(totalLines, (size_t)(sourceLocation.line + 1));

  const size_t maxDigits = (maxLineNumber == 0) ? 1 : (log10(maxLineNumber) + 1);

  auto line = sourceManager.getLine(sourceLocation);
  assert(sourceLocation.column < line.size());

  std::string caret(sourceLocation.column, ' ');
  for (size_t index = 0; index < sourceLocation.column; index++) {
    if (line[index] == '\t') {
      caret[index] = '\t';
    }
  }
  caret[sourceLocation.column - 1] = '^';
  caret.insert(0, std::string(maxDigits + delimiter.size(), ' '));

  if (sourceLocation.line > 1) {
    SourceLocation previousLineLocation(sourceLocation.unitDirectory,
                                        sourceLocation.unitFilePath,
                                        sourceLocation.directory,
                                        sourceLocation.filePath,
                                        sourceLocation.line - 1,
                                        sourceLocation.column);
    auto previousLine = sourceManager.getLine(previousLineLocation);
    ss << std::setw(maxDigits) << previousLineLocation.line << delimiter << previousLine;
  }

  ss << std::setw(maxDigits) << sourceLocation.line << delimiter << line << caret << "\n";

  if (sourceLocation.line < totalLines) {
    SourceLocation nextLineLocation(sourceLocation.unitDirectory,
                                    sourceLocation.unitFilePath,
                                    sourceLocation.directory,
                                    sourceLocation.filePath,
                                    sourceLocation.line + 1,
                                    sourceLocation.column);
    auto nextLine = sourceManager.getLine(nextLineLocation);
    ss << std::setw(maxDigits) << nextLineLocation.line << delimiter << nextLine;
  }

  return ss.str();
}

std::string SourceCodeReader::getSourceLineWithCaret(const SourceLocation &sourceLocation) {
  std::stringstream ss;

  auto line = sourceManager.getLine(sourceLocation);
  assert(sourceLocation.column < line.size());

  std::string caret(sourceLocation.column, ' ');
  for (size_t index = 0; index < sourceLocation.column; index++) {
    if (line[index] == '\t') {
      caret[index] = '\t';
    }
  }
  caret[sourceLocation.column - 1] = '^';

  ss << line << caret << "\n";
  return ss.str();
}

std::vector<std::string> SourceCodeReader::getSourceLines(const SourceLocation &sourceLocation,
                                                          const SourceLocation &sourceEndLocation) {
  std::vector<std::string> lines;
  for (SourceLocation temp{ sourceLocation }; temp.line <= sourceEndLocation.line; temp.line++) {
    lines.push_back(sourceManager.getLine(temp));
  }
  return lines;
}
