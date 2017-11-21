#include "TestRunner.h"

#include <llvm/Support/DynamicLibrary.h>

using namespace mull;
using namespace llvm;

TestRunner::TestRunner(llvm::TargetMachine &targetMachine)
  : machine(targetMachine)
{
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}
