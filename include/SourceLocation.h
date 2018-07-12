#pragma once

#include <string>

namespace llvm {
class Instruction;
class Function;
}

namespace mull {

struct SourceLocation {
  std::string directory;
  std::string filePath;
  int line;
  int column;
  SourceLocation(const std::string &directory, const std::string &filePath, int line, int column);
  bool isNull() const;

  const static SourceLocation sourceLocationFromInstruction(llvm::Instruction *instruction);
  const static SourceLocation sourceLocationFromFunction(llvm::Function *function);
  const static SourceLocation nullSourceLocation();
};

}
