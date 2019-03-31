#pragma once

#include <llvm/Support/raw_ostream.h>

namespace mull {

class Logger {
public:
  Logger() = delete;

  enum class Level { error, warn, info, debug };

  static void setLevel(Logger::Level level);

  static llvm::raw_ostream &error();
  static llvm::raw_ostream &warn();
  static llvm::raw_ostream &info();
  static llvm::raw_ostream &debug();

private:
  static llvm::raw_ostream &getStream(Logger::Level level);
  static Level logLevel;
};
} // namespace mull
