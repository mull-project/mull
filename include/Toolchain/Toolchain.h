#pragma once

#include "Toolchain/ObjectCache.h"

namespace Mutang {
  class Config;

  class Toolchain {
    ObjectCache objectCache;
  public:
    Toolchain(Config &config);

    ObjectCache &cache();
  };
}
