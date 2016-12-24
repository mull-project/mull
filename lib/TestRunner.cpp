#include "TestRunner.h"

#include "llvm/ADT/Triple.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/TargetSelect.h"

using namespace mull;
using namespace llvm;

TestRunner::TestRunner(llvm::TargetMachine &targetMachine)
  : machine(targetMachine)
{
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  LLVMLinkInOrcMCJITReplacement();
}
