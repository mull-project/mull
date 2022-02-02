#include "mull/Diagnostics/Diagnostics.h"
#include <iostream>
#include <memory>

using namespace mull;

Diagnostics::Diagnostics()
    : seenProgress(false), debugModeEnabled(false), strictModeEnabled(false), quiet(false),
      silent(false) {}

void Diagnostics::enableDebugMode() {
  std::lock_guard<std::mutex> guard(mutex);
  debugModeEnabled = true;
}

void Diagnostics::enableStrictMode() {
  std::lock_guard<std::mutex> guard(mutex);
  strictModeEnabled = true;
}

void Diagnostics::makeQuiet() {
  std::lock_guard<std::mutex> guard(mutex);
  quiet = true;
}

void Diagnostics::makeSilent() {
  std::lock_guard<std::mutex> guard(mutex);
  silent = true;
}

void Diagnostics::info(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  if (quiet) {
    return;
  }
  prepare();
  std::cout << "[info] " << message << std::endl;
}

void Diagnostics::warning(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  if (silent && !strictModeEnabled) {
    return;
  }
  prepare();
  std::cout << "[warning] " << message << std::endl;
  if (strictModeEnabled) {
    std::cout << "[warning] Strict Mode enabled: warning messages are treated as fatal errors. "
                 "Exiting now."
              << std::endl;
    exit(1);
  }
}

void Diagnostics::error(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  prepare();
  std::cerr << "[error] " << message << std::endl;
  std::cerr << "[error] Error messages are treated as fatal errors. Exiting now." << std::endl;
  exit(1);
}

void Diagnostics::progress(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  if (quiet) {
    return;
  }
  seenProgress = true;
  std::cout << message;
}

void Diagnostics::debug(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  if (!debugModeEnabled) {
    return;
  }
  prepare();
  std::cout << "[debug] " << message << std::endl;
}

void Diagnostics::prepare() {
  if (seenProgress) {
    std::cout << std::endl;
    seenProgress = false;
  }
}
