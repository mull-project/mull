#pragma once

#include "mull/Program/Program.h"

namespace mull {

class Configuration;

class ObjectLoader {
public:
  ObjectFiles loadObjectFiles(Configuration &configuration, Diagnostics &diagnostics);

private:
};

} // namespace mull
