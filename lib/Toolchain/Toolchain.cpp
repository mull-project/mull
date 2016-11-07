#include "Toolchain/Toolchain.h"

using namespace Mutang;

Toolchain::Toolchain() {}

ObjectCache &Toolchain::cache() {
  return objectCache;
}
