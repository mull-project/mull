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

void *SimpleTestRunner::TestFunctionPointer(const llvm::Function &Function) {
  llvm_compat::JITSymbol &symbol = jit.getSymbol(mangler.getNameWithPrefix(Function.getName()));
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(llvm_compat::JITSymbolAddress(symbol)));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

void SimpleTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation) {
  InstrumentationResolver resolver(overrides, instrumentation, mangler, trampoline);
  jit.addObjectFiles(objectFiles, resolver, make_unique<SectionMemoryManager>());
}

void SimpleTestRunner::loadProgram(ObjectFiles &objectFiles) {
  NativeResolver resolver(overrides);
  jit.addObjectFiles(objectFiles, resolver, make_unique<SectionMemoryManager>());
}

ExecutionStatus SimpleTestRunner::runTest(Test *test) {
  *trampoline = &test->getInstrumentationInfo();
  assert(isa<SimpleTest_Test>(test) && "Supposed to work only with");

  SimpleTest_Test *SimpleTest = dyn_cast<SimpleTest_Test>(test);

  void *FunctionPointer = TestFunctionPointer(*SimpleTest->GetTestFunction());

  uint64_t result = ((int (*)())(intptr_t)FunctionPointer)();

  overrides.runDestructors();

  if (result == 1) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
