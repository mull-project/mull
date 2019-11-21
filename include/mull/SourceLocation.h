#pragma once

#include <string>

namespace llvm {
class Instruction;
class Function;
} // namespace llvm

namespace mull {

struct SourceLocation {
  std::string unitDirectory;
  std::string unitFilePath;
  std::string directory;
  std::string filePath;
  int line;
  int column;
  SourceLocation(std::string unitDirectory, std::string unitFilePath, std::string directory,
                 std::string filePath, int line, int column);
  bool isNull() const;

  static SourceLocation locationFromInstruction(const llvm::Instruction *instruction);
  static SourceLocation locationFromFunction(const llvm::Function *function);
  static SourceLocation nullSourceLocation();
};

} // namespace mull
