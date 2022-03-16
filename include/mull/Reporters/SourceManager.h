#pragma once

#include "mull/SourceLocation.h"

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
  size_t getNumberOfLines(const SourceLocation &location);

  ~SourceManager();

private:
  std::map<std::string, LineOffset> lineOffsets;

  LineOffset &getLineOffset(const SourceLocation &location);
};

} // namespace mull
