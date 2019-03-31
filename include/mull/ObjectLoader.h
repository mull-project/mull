#pragma once

#include "mull/Program/Program.h"

namespace mull {

class Configuration;

class ObjectLoader {
public:
  ObjectFiles loadObjectFiles(Configuration &configuration);

private:
};

} // namespace mull
