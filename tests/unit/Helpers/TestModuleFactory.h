#include "mull/Bitcode.h"

using namespace llvm;
using namespace mull;

namespace mull {
std::unique_ptr<Bitcode> loadBitcodeFromIR(const char *path);
}
