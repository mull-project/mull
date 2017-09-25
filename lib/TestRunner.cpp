#include "TestRunner.h"

#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/Support/DynamicLibrary.h"

using namespace mull;
using namespace llvm;

TestRunner::TestRunner(TargetMachine &machine, MullJIT &jit) :
    targetMachine(machine), jit(jit)
{
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  LLVMLinkInOrcMCJITReplacement();
}
