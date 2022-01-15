#pragma once

#include <string>
#include <mutex>

namespace mull {

class DiagnosticsImpl;

class Diagnostics {
public:
  Diagnostics();
  ~Diagnostics();

  void enableDebugMode();
  void enableStrictMode();
  void makeQuiet();
  void makeSilent();

  void info(const std::string& message);
  void warning(const std::string& message);
  void error(const std::string& message);
  void progress(const std::string& message);
  void debug(const std::string& message);

private:
  void prepare();

  DiagnosticsImpl *impl;
  std::mutex mutex;
  bool seenProgress;
  bool debugModeEnabled;
  bool strictModeEnabled;
  bool quiet;
  bool silent;
};

}
