#include "mull/Reporters/SourceManager.h"

#include <cassert>

using namespace mull;

SourceManager::~SourceManager() {
  for (auto &pair : lineOffsets) {
    fclose(pair.second.file);
  }
}

std::string SourceManager::getLine(const SourceLocation &location) {
  LineOffset &lineOffset = getLineOffset(location);
  assert(location.line < lineOffset.offsets.size());

  uint32_t lineBegin = lineOffset.offsets[location.line - 1];
  uint32_t lineEnd = lineOffset.offsets[location.line];
  uint32_t lineLength = lineEnd - lineBegin;
  fseek(lineOffset.file, lineBegin, SEEK_SET);
  char *buffer = new char[lineLength + 1];
  fread(buffer, sizeof(char), lineLength, lineOffset.file);
  buffer[lineLength] = '\0';
  auto line = std::string(buffer);
  delete[] buffer;
  return line;
}

size_t SourceManager::getNumberOfLines(const SourceLocation &location) {
  const LineOffset &lineOffset = getLineOffset(location);
  assert(lineOffset.offsets.size() > 0);
  return lineOffset.offsets.size() - 1;
}

LineOffset &SourceManager::getLineOffset(const SourceLocation &location) {
  if (lineOffsets.count(location.filePath)) {
    return lineOffsets.at(location.filePath);
  }

  FILE *file = fopen(location.filePath.c_str(), "rb");
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
  auto inserted = lineOffsets.insert(std::make_pair(location.filePath, lineOffset));
  return inserted.first->second;
}
