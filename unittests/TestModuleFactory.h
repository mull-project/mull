#include "MullModule.h"

using namespace llvm;
using namespace mull;

namespace mull {
std::unique_ptr<MullModule> loadModuleFromIR(const char *path, LLVMContext &context);
}
