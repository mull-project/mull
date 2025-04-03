#pragma once

#include "mull/SourceLocation.h"

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace mull {

struct LineOffset {
  FILE *file;
  std::vector<uint32_t> offsets;
  LineOffset(FILE *file, std::vector<uint32_t> offsets) : file(file), offsets(std::move(offsets)) {}
};

class SourceManager {
public:
  std::string getLine(const SourceLocation &location);
  std::string getLine(const std::string &filePath, size_t lineNumber);
  size_t getNumberOfLines(const SourceLocation &location);
  size_t getNumberOfLines(const std::string &filePath);

  ~SourceManager();

private:
  std::map<std::string, LineOffset> lineOffsets;

  LineOffset &getLineOffset(const SourceLocation &location);
  LineOffset &getLineOffset(const std::string &filePath);
};

} // namespace mull
