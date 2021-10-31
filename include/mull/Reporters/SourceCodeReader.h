#pragma once

#include "mull/Reporters/SourceManager.h"

#include <string>

namespace mull {

struct SourceLocation;

class SourceCodeReader {
public:
  SourceCodeReader();
  std::string getContext(const mull::SourceLocation &sourceLocation);
  std::string getSourceLineWithCaret(const SourceLocation &sourceLocation);
  std::string getSourceLine(const SourceLocation &sourceLocation);
private:
  SourceManager sourceManager;
};
} // namespace mull
