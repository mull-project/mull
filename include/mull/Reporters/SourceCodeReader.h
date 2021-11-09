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
  std::vector<std::string> getSourceLines(const SourceLocation &sourceLocation, const SourceLocation &sourceEndLocation);
private:
  SourceManager sourceManager;
};
} // namespace mull
