#include "mull/Diagnostics/Diagnostics.h"
#include <memory>
#include <unistd.h>

using namespace mull;

static const char *levels[] = { "info", "warning", "error", "debug" };
static const char *colors[] = { "\033[32m", "\033[33m", "\033[31m" };

Diagnostics::Diagnostics()
    : seenProgress(false), showColor(false), hasTerm(false), debugModeEnabled(false),
      output(stdout) {
  hasTerm = getenv("TERM") != nullptr;
  showColor = isatty(fileno(output)) && hasTerm;
}

void Diagnostics::log(LogLevel level, const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  interceptProgress();
  fputs("[", output);
  if (showColor) {
    fputs(colors[level], output);
  }
  fputs(levels[level], output);
  if (showColor) {
    fputs("\033[0m", output);
  }
  fputs("] ", output);
  fputs(message.c_str(), output);
  fputs("\n", output);
  fflush(output);
}

void Diagnostics::enableDebugMode() {
  debugModeEnabled = true;
}

void Diagnostics::info(const std::string &message) {
  log(LogLevel::Info, message);
}

void Diagnostics::warning(const std::string &message) {
  log(LogLevel::Warning, message);
}

void Diagnostics::error(const std::string &message) {
  log(LogLevel::Error, message);
}

void Diagnostics::progress(const std::string &message, bool clear) {
  std::lock_guard<std::mutex> guard(mutex);
  seenProgress = true;
  if (clear && hasTerm) {
    fputs("\r", output);
  }
  fputs(message.c_str(), output);
  fflush(output);
}

void Diagnostics::debug(const std::string &message) {
  if (!debugModeEnabled) {
    return;
  }
  log(LogLevel::Debug, message);
}

void Diagnostics::interceptProgress() {
  if (seenProgress) {
    fputs("\n", output);
    fflush(output);
    seenProgress = false;
  }
}
