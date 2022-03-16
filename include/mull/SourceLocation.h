#pragma once

#include <string>

namespace llvm {
class Instruction;
class Function;
} // namespace llvm

namespace mull {

struct SourceLocation {
  /// The difference between (directory, filePath) and (unitDirectory, unitFilePath)
  /// is as follows:
  /// Both paths are the same when a mutation is located in the .c/.cpp file.
  /// When a mutation is inlined into a preprocessed .c/.cpp file from a header file:
  /// - the directory and filePath point to the location of the mutation in the
  ///   inlined header file.
  /// - the unitDirectory and unitFilePath point to the location of the mutation
  ///   in the .c/.cpp file.
  std::string unitDirectory;
  std::string unitFilePath;
  std::string directory;
  std::string filePath;

  /// Line and columns numbers are 1-based.
  /// 0 value indicates missing (junk mutation) or broken information.
  size_t line;
  size_t column;
  SourceLocation(std::string unitDirectory, std::string unitFilePath, std::string directory,
                 std::string filePath, size_t line, size_t column);
  bool isNull() const;
  bool canRead() const;

  static SourceLocation locationFromInstruction(const llvm::Instruction *instruction);
  static SourceLocation locationFromFunction(const llvm::Function *function);
  static SourceLocation nullSourceLocation();
};

} // namespace mull
