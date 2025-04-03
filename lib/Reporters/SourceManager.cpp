#include "mull/Reporters/SourceManager.h"

#include <cassert>

using namespace mull;

SourceManager::~SourceManager() {
  for (auto &pair : lineOffsets) {
    fclose(pair.second.file);
  }
}

std::string SourceManager::getLine(const std::string &filePath, size_t lineNumber) {
  LineOffset &lineOffset = getLineOffset(filePath);
  assert(lineNumber < lineOffset.offsets.size());

  uint32_t lineBegin = lineOffset.offsets[lineNumber - 1];
  uint32_t lineEnd = lineOffset.offsets[lineNumber];
  uint32_t lineLength = lineEnd - lineBegin;
  fseek(lineOffset.file, lineBegin, SEEK_SET);
  char *buffer = new char[lineLength + 1];
  fread(buffer, sizeof(char), lineLength, lineOffset.file);
  buffer[lineLength] = '\0';
  auto line = std::string(buffer);
  delete[] buffer;
  return line;
}

std::string SourceManager::getLine(const SourceLocation &location) {
  return getLine(location.filePath, location.line);
}

size_t SourceManager::getNumberOfLines(const SourceLocation &location) {
  return getNumberOfLines(location.filePath);
}

size_t SourceManager::getNumberOfLines(const std::string &filePath) {
  const LineOffset &lineOffset = getLineOffset(filePath);
  assert(lineOffset.offsets.size() > 0);
  return lineOffset.offsets.size() - 1;
}

LineOffset &SourceManager::getLineOffset(const SourceLocation &location) {
  return getLineOffset(location.filePath);
}

LineOffset &SourceManager::getLineOffset(const std::string &filePath) {
  if (lineOffsets.count(filePath)) {
    return lineOffsets.at(filePath);
  }

  FILE *file = fopen(filePath.c_str(), "rb");
  if (!file) {
    perror("SourceManager");
  }
  std::vector<uint32_t> offsets;
  uint32_t offset = 0;
  int c = '\n';
  for (; c != EOF; c = fgetc(file), offset++) {
    if (c == '\n') {
      offsets.push_back(offset);
    }
  }
  offsets.push_back(offset);

  LineOffset lineOffset(file, offsets);
  auto inserted = lineOffsets.insert(std::make_pair(filePath, lineOffset));
  return inserted.first->second;
}
