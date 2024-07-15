#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

#include "GeneratedConfiguration.h"

extern int MullDefaultTimeoutMilliseconds;

class Diagnostics;

struct Configuration : public GeneratedConfiguration {
  std::string pathOnDisk;
  bool captureMutantOutput;
  unsigned timeout;

  std::string executable;

  Configuration();

  static std::string findConfig(Diagnostics &diagnostics);
  static Configuration loadFromDisk(Diagnostics &diagnostics, const std::string &path);
};

} // namespace mull
