#pragma once

#include "mull/Reporters/SourceManager.h"

#include <string>

namespace mull {

class SourceLocation;

class SourceCodeReader {
public:
  SourceCodeReader();
  std::string getContext(const mull::SourceLocation &sourceLocation);
  std::string getSourceLineWithCaret(const SourceLocation &sourceLocation);
private:
  SourceManager sourceManager;
};
} // namespace mull
