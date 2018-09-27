#include "GoogleTest/GoogleTestRunner.h"

#include "GoogleTest/GoogleTest_Test.h"
#include "Mangler.h"

#include "Toolchain/Resolvers/InstrumentationResolver.h"
#include "Toolchain/Resolvers/NativeResolver.h"

#include <llvm/IR/Function.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

using namespace mull;
using namespace llvm;
using namespace llvm::orc;

namespace {
  class UnitTest;
}

GoogleTestRunner::GoogleTestRunner(Mangler &mangler) :
  mangler(mangler),
  overrides([this](const char *name) {
    return this->mangler.getNameWithPrefix(name);
  }),
  fGoogleTestInit(mangler.getNameWithPrefix("_ZN7testing14InitGoogleTestEPiPPc")),
  fGoogleTestInstance(mangler.getNameWithPrefix("_ZN7testing8UnitTest11GetInstanceEv")),
  fGoogleTestRun(mangler.getNameWithPrefix("_ZN7testing8UnitTest3RunEv")),
  trampoline(new InstrumentationInfo*)
{
}

GoogleTestRunner::~GoogleTestRunner() {
  delete trampoline;
}

void *GoogleTestRunner::getConstructorPointer(const llvm::Function &function,
                                              JITEngine &jit) {
  auto name = mangler.getNameWithPrefix(function.getName().str());
  return getFunctionPointer(name, jit);
}

void *GoogleTestRunner::getFunctionPointer(const std::string &functionName,
                                           JITEngine &jit) {
  JITSymbol &symbol = jit.getSymbol(functionName);
  auto address = llvm_compat::JITSymbolAddress(symbol);

  void *pointer = reinterpret_cast<void *>(static_cast<uintptr_t>(address));

  if (pointer == nullptr) {
    errs() << "GoogleTestRunner> Can't find pointer to function: "
           << functionName << "\n";
    exit(1);
  }

  return pointer;
}

void GoogleTestRunner::runStaticConstructor(llvm::Function *constructor,
                                            JITEngine &jit) {
//  printf("Init: %s\n", Ctor->getName().str().c_str());

  void *CtorPointer = getConstructorPointer(*constructor, jit);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

void GoogleTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation,
                                               JITEngine &jit) {
  InstrumentationResolver resolver(overrides, instrumentation, mangler, trampoline);
  jit.addObjectFiles(objectFiles, resolver, make_unique<SectionMemoryManager>());
}

void GoogleTestRunner::loadProgram(ObjectFiles &objectFiles, JITEngine &jit) {
  NativeResolver resolver(overrides);
  jit.addObjectFiles(objectFiles, resolver, make_unique<SectionMemoryManager>());
}

ExecutionStatus GoogleTestRunner::runTest(Test *test, JITEngine &jit) {
  *trampoline = &test->getInstrumentationInfo();

  GoogleTest_Test *GTest = dyn_cast<GoogleTest_Test>(test);

  for (auto &Ctor: GTest->GetGlobalCtors()) {
    runStaticConstructor(Ctor, jit);
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

  void *initGTestPtr = getFunctionPointer(fGoogleTestInit, jit);

  auto initGTest = ((void (*)(int *, const char**))(intptr_t)initGTestPtr);
  initGTest(&argc, argv);

  void *getInstancePtr = getFunctionPointer(fGoogleTestInstance, jit);

  auto getInstance = ((UnitTest *(*)())(intptr_t)getInstancePtr);
  UnitTest *unitTest = getInstance();

  void *runAllTestsPtr = getFunctionPointer(fGoogleTestRun, jit);

  auto runAllTests = ((int (*)(UnitTest *))(intptr_t)runAllTestsPtr);
  uint64_t result = runAllTests(unitTest);

  overrides.runDestructors();

  if (result == 0) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
