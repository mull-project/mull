#include "mull/Version.h"

#include <llvm/Support/raw_ostream.h>

namespace mull {

const char *mullVersionString() {
  return "@PROJECT_VERSION@";
}
const char *mullHomepageString() {
  return "@PROJECT_HOMEPAGE_URL@";
}
const char *llvmVersionString() {
  return "@LLVM_VERSION@";
}

} // namespace mull
