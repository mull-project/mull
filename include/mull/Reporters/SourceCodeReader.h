#pragma once

#include "mull/Reporters/SourceManager.h"

#include <string>

namespace mull {

class SourceLocation;

class SourceCodeReader {
public:
  SourceCodeReader();
  std::string getContext(const mull::SourceLocation &sourceLocation);
private:
  SourceManager sourceManager;
};
} // namespace mull
