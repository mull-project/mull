#pragma once

#include <mutex>
#include <string>

namespace mull {

class Diagnostics {
public:
  enum LogLevel { Info, Warning, Error, Debug };

  Diagnostics();

  void enableDebugMode();
  void enableStrictMode();

  void info(const std::string& message);
  void warning(const std::string& message);
  void error(const std::string& message);
  void progress(const std::string &message, bool clear = true);
  void debug(const std::string& message);

private:
  void log(LogLevel level, const std::string &message);
  void interceptProgress();

  std::mutex mutex;
  bool seenProgress;
  bool showColor;
  bool hasTerm;
  bool debugModeEnabled;
  bool strictModeEnabled;
  FILE *output;
};

} // namespace mull
