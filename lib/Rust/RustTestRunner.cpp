#include "Rust/RustTestRunner.h"

#include "Config.h"
#include "Logger.h"
#include "ForkProcessSandbox.h"
#include "Rust/RustTest.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/TargetSelect.h"

#include <chrono>
#include <dlfcn.h>
#include <execinfo.h>

using namespace mull;
using namespace llvm;
using namespace std::chrono;

class Mull_Rust_Resolver : public RuntimeDyld::SymbolResolver {
public:

  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) {
    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);
    }

    return RuntimeDyld::SymbolInfo(nullptr);
  }

  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &Name) {
    return RuntimeDyld::SymbolInfo(nullptr);
  }
};

RustTestRunner::RustTestRunner(llvm::TargetMachine &machine)
  : TestRunner(machine) {}

void *RustTestRunner::FunctionPointer(const char *functionName) {
  orc::JITSymbol symbol = objectLayer.findSymbol(functionName, false);

  void *FPointer =
    reinterpret_cast<void *>(static_cast<uintptr_t>(symbol.getAddress()));

  assert(FPointer && "Can't find pointer to function");

  return FPointer;
}

ExecutionResult RustTestRunner::runTest(Test *Test, ObjectFiles &objectFiles) {
  RustTest *rustTest = dyn_cast<RustTest>(Test);

  auto handle = objectLayer.addObjectSet(objectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<Mull_Rust_Resolver>());

  auto start = high_resolution_clock::now();

  std::string rustTestFunctionName = std::string("_") + rustTest->getTestName();
  void *rustTestPointer = FunctionPointer(rustTestFunctionName.c_str());

  // Normally Rust tests are run via `main` function which calls
  // test_main_static function under the hood.
  // We want to run tests 1 by 1 so we don't have Rust's exception handlers set
  // for us: if a single Rust test hits failing assertion, it panics ending up
  // in abort() call and a signal that we recognize as having Crashed status.
  // So two different groups are under Crashed umbrella:
  // - Failing assertions.
  // - All other program crashes that happen because of corruption produced by
  // mutation operators.
    auto rustTestFunction = ((int (*)())(intptr_t)rustTestPointer);
  __unused auto res = rustTestFunction();

  auto elapsed = high_resolution_clock::now() - start;

  ExecutionResult Result;
  Result.Status = Passed;
  Result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

  objectLayer.removeObjectSet(handle);

  return Result;
}
