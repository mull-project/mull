#include "mull/Diagnostics/Diagnostics.h"
#include <memory>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>

using namespace mull;

class mull::DiagnosticsImpl {
public:
  DiagnosticsImpl() {
    logger = std::unique_ptr<spdlog::logger>(new spdlog::logger(
        "default", spdlog::sink_ptr(new spdlog::sinks::ansicolor_stdout_sink_st)));
    logger->set_pattern("[%^%l%$] %v");
  }
  spdlog::logger &log() {
    return *logger;
  }

private:
  std::unique_ptr<spdlog::logger> logger;
};

Diagnostics::Diagnostics() : impl(new DiagnosticsImpl()), seenProgress(false) {}

Diagnostics::~Diagnostics() {
  delete impl;
}

void Diagnostics::info(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  prepare();
  impl->log().info(message);
}

void Diagnostics::warning(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  prepare();
  impl->log().warn(message);
}

void Diagnostics::error(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  prepare();
  impl->log().error(message);
}

void Diagnostics::progress(const std::string &message) {
  std::lock_guard<std::mutex> guard(mutex);
  seenProgress = true;
  fprintf(stdout, "%s", message.c_str());
  fflush(stdout);
}

void Diagnostics::prepare() {
  if (seenProgress) {
    fprintf(stdout, "\n");
    fflush(stdout);
    seenProgress = false;
  }
}
