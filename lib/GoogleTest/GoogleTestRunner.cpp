#include "GoogleTest/GoogleTestRunner.h"

#include "GoogleTest/GoogleTest_Test.h"
#include "Mangler.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

using namespace mull;
using namespace llvm;
using namespace llvm::orc;

namespace {
  class UnitTest;
}

static llvm::orc::ObjectLinkingLayer<>::ObjSetHandleT MullGTEstDummyHandle;

class Mull_GoogleTest_Resolver : public RuntimeDyld::SymbolResolver {
  LocalCXXRuntimeOverrides &overrides;
  std::string instrumentationInfoName;
  InstrumentationInfo **trampoline;
public:
  Mull_GoogleTest_Resolver(LocalCXXRuntimeOverrides &overrides,
                           std::string instrumentationInfoName,
                           InstrumentationInfo **trampoline)
    : overrides(overrides), instrumentationInfoName(instrumentationInfoName), trampoline(trampoline) {}

  RuntimeDyld::SymbolInfo findSymbol(const std::string &name) {
    /// Overrides should go first, otherwise functions of the host process
    /// will take over and crash the system later
    if (auto symbol = overrides.searchOverrides(name)) {
      return symbol;
    }

    if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
      return RuntimeDyld::SymbolInfo(address, JITSymbolFlags::Exported);
    }

    if (name == instrumentationInfoName) {
      return RuntimeDyld::SymbolInfo((uint64_t)trampoline, JITSymbolFlags::Exported);
    }

    return RuntimeDyld::SymbolInfo(nullptr);
  }

  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &name) {
    return RuntimeDyld::SymbolInfo(nullptr);
  }
};

GoogleTestRunner::GoogleTestRunner(llvm::TargetMachine &machine) :
  TestRunner(machine),
  mangler(Mangler(machine.createDataLayout())),
  overrides([this](const char *name) {
    return this->mangler.getNameWithPrefix(name);
  }),
  fGoogleTestInit(mangler.getNameWithPrefix("_ZN7testing14InitGoogleTestEPiPPc")),
  fGoogleTestInstance(mangler.getNameWithPrefix("_ZN7testing8UnitTest11GetInstanceEv")),
  fGoogleTestRun(mangler.getNameWithPrefix("_ZN7testing8UnitTest3RunEv")),
  instrumentationInfoName(mangler.getNameWithPrefix("mull_instrumentation_info")),
  handle(MullGTEstDummyHandle),
  trampoline(new InstrumentationInfo*)
{
}

GoogleTestRunner::~GoogleTestRunner() {
  delete trampoline;
}

void *GoogleTestRunner::GetCtorPointer(const llvm::Function &Function) {
  return
    getFunctionPointer(mangler.getNameWithPrefix(Function.getName().str()));
}

void *GoogleTestRunner::getFunctionPointer(const std::string &functionName) {
  JITSymbol symbol = ObjectLayer.findSymbol(functionName, false);

  void *fpointer =
    reinterpret_cast<void *>(static_cast<uintptr_t>(symbol.getAddress()));

  if (fpointer == nullptr) {
    errs() << "GoogleTestRunner> Can't find pointer to function: "
           << functionName << "\n";
    exit(1);
  }

  return fpointer;
}

void GoogleTestRunner::runStaticCtor(llvm::Function *Ctor) {
//  printf("Init: %s\n", Ctor->getName().str().c_str());

  void *CtorPointer = GetCtorPointer(*Ctor);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

void GoogleTestRunner::loadProgram(ObjectFiles &objectFiles) {
  if (handle != MullGTEstDummyHandle) {
    ObjectLayer.removeObjectSet(handle);
  }

  handle = ObjectLayer.addObjectSet(objectFiles,
                                    make_unique<SectionMemoryManager>(),
                                    make_unique<Mull_GoogleTest_Resolver>(overrides,
                                                                          instrumentationInfoName,
                                                                          trampoline));
  ObjectLayer.emitAndFinalize(handle);
}

ExecutionStatus GoogleTestRunner::runTest(Test *test) {
  *trampoline = &test->getInstrumentationInfo();

  GoogleTest_Test *GTest = dyn_cast<GoogleTest_Test>(test);

  for (auto &Ctor: GTest->GetGlobalCtors()) {
    runStaticCtor(Ctor);
  }

  std::string filter = "--gtest_filter=" + GTest->getTestName();
  const char *argv[] = { "mull", filter.c_str(), NULL };
  int argc = 2;

  /// Normally Google Test Driver looks like this:
  ///
  ///   int main(int argc, char **argv) {
  ///     InitGoogleTest(&argc, argv);
  ///     return UnitTest.GetInstance()->Run();
  ///   }
  ///
  /// Technically we can just call `main` function, but there is a problem:
  /// Among all the files that are being processed may be more than one
  /// `main` function, therefore we can call wrong driver.
  ///
  /// To avoid this from happening we implement the driver function on our own.
  /// We must keep in mind that each project can have its own, extended
  /// version of the driver (LLVM itself has one).
  ///

  void *initGTestPtr = getFunctionPointer(fGoogleTestInit);

  auto initGTest = ((void (*)(int *, const char**))(intptr_t)initGTestPtr);
  initGTest(&argc, argv);

  void *getInstancePtr = getFunctionPointer(fGoogleTestInstance);

  auto getInstance = ((UnitTest *(*)())(intptr_t)getInstancePtr);
  UnitTest *unitTest = getInstance();

  void *runAllTestsPtr = getFunctionPointer(fGoogleTestRun);

  auto runAllTests = ((int (*)(UnitTest *))(intptr_t)runAllTestsPtr);
  uint64_t result = runAllTests(unitTest);

  overrides.runDestructors();

  if (result == 0) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
