#include "Toolchain/Toolchain.h"

#include "Config.h"

using namespace Mutang;

Toolchain::Toolchain(Config &config)
  : objectCache(config.getUseCache(), config.getCacheDirectory())
{
}

ObjectCache &Toolchain::cache() {
  return objectCache;
}
