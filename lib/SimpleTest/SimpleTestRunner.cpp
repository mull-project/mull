#include "SimpleTest/SimpleTestRunner.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "Toolchain/Resolvers/InstrumentationResolver.h"
#include "Toolchain/Resolvers/NativeResolver.h"
#include "Mangler.h"

#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Function.h>

#include <string>

using namespace mull;
using namespace llvm;

SimpleTestRunner::SimpleTestRunner(TargetMachine &machine)
  : TestRunner(machine),
    mangler(Mangler(machine.createDataLayout())),
    overrides([this](const char *name) {
      return this->mangler.getNameWithPrefix(name);
    }),
    trampoline(new InstrumentationInfo*)
{}

SimpleTestRunner::~SimpleTestRunner() {
  delete trampoline;
}

void *SimpleTestRunner::functionPointer(const llvm::Function &function,
                                        JITEngine &jit) {
  auto mangledName = mangler.getNameWithPrefix(function.getName());
  llvm_compat::JITSymbol &symbol = jit.getSymbol(mangledName);
  auto address = llvm_compat::JITSymbolAddress(symbol);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(address));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

void SimpleTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation,
                                               JITEngine &jit) {
  InstrumentationResolver resolver(overrides, instrumentation, mangler, trampoline);
  jit.addObjectFiles(objectFiles, resolver, make_unique<SectionMemoryManager>());
}

void SimpleTestRunner::loadProgram(ObjectFiles &objectFiles, JITEngine &jit) {
  NativeResolver resolver(overrides);
  jit.addObjectFiles(objectFiles, resolver, make_unique<SectionMemoryManager>());
}

ExecutionStatus SimpleTestRunner::runTest(Test *test, JITEngine &jit) {
  *trampoline = &test->getInstrumentationInfo();
  assert(isa<SimpleTest_Test>(test) && "Supposed to work only with");

  SimpleTest_Test *SimpleTest = dyn_cast<SimpleTest_Test>(test);

  void *FunctionPointer = functionPointer(*SimpleTest->GetTestFunction(),
                                          jit);

  uint64_t result = ((int (*)())(intptr_t)FunctionPointer)();

  overrides.runDestructors();

  if (result == 1) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
