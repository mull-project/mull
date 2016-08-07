#include "TestRunners/SimpleTestRunner.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/TargetSelect.h"

#include <chrono>

using namespace Mutang;
using namespace llvm;
using namespace std::chrono;

class MutangResolver : public JITSymbolResolver {
public:

  JITSymbol findSymbol(const std::string &Name) {
    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
      return JITSymbol(SymAddr, JITSymbolFlags::Exported);
    return JITSymbol(nullptr);
  }

  JITSymbol findSymbolInLogicalDylib(const std::string &Name) {
    return JITSymbol(nullptr);
  }
};

SimpleTestRunner::SimpleTestRunner() : TM(EngineBuilder().selectTarget(
                                            Triple(), "", "",
                                            SmallVector<std::string, 1>())) {
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  LLVMLinkInOrcMCJITReplacement();
}

std::string SimpleTestRunner::MangleName(const llvm::StringRef &Name) {
  std::string MangledName;
  {
    raw_string_ostream Stream(MangledName);
    Mangler.getNameWithPrefix(Stream, Name, TM->createDataLayout());
  }
  return MangledName;
}

void *SimpleTestRunner::TestFunctionPointer(const llvm::Function &Function) {
  JITSymbol Symbol = ObjectLayer.findSymbol(MangleName(Function.getName()), true);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(Symbol.getAddress()));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

ExecutionResult SimpleTestRunner::runTest(llvm::Function *Test,
                                          ObjectFiles &ObjectFiles) {
  auto Handle = ObjectLayer.addObjectSet(ObjectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<MutangResolver>());
  void *FunctionPointer = TestFunctionPointer(*Test);

  auto start = high_resolution_clock::now();
  uint64_t result = ((int (*)())(intptr_t)FunctionPointer)();
  auto elapsed = high_resolution_clock::now() - start;

  ExecutionResult Result;
  Result.RunningTime = duration_cast<std::chrono::nanoseconds>(elapsed).count();

  ObjectLayer.removeObjectSet(Handle);

  if (result == 1) {
    Result.Status = ExecutionStatus::Passed;
  } else {
    Result.Status = ExecutionStatus::Failed;
  }

  return Result;
}
