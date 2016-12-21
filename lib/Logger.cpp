#include "Logger.h"

namespace Mutang {

void Logger::setLevel(Logger::Level level) { logLevel = level; }

llvm::raw_ostream &Logger::error() { return getStream(Logger::Level::error); }
llvm::raw_ostream &Logger::warn() { return getStream(Logger::Level::warn); }
llvm::raw_ostream &Logger::info() { return getStream(Logger::Level::info); }
llvm::raw_ostream &Logger::debug() { return getStream(Logger::Level::debug); }

llvm::raw_ostream &Logger::getStream(Logger::Level level) {
  if (Logger::logLevel <= level)
    return llvm::outs();
  return llvm::nulls();
}

// Initialize logger to info.
Logger::Level Logger::logLevel = Logger::Level::info;
}
