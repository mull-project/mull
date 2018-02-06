#include "SimpleTest/SimpleTestRunner.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "Toolchain/Resolvers/InstrumentationResolver.h"
#include "Toolchain/Resolvers/NativeResolver.h"
#include "Mangler.h"

#include <llvm/ExecutionEngine/SectionMemoryManager.h>

#include <string>

using namespace mull;
using namespace llvm;

static llvm::orc::ObjectLinkingLayer<>::ObjSetHandleT MullSimpleTestDummyHandle;

SimpleTestRunner::SimpleTestRunner(TargetMachine &machine)
  : TestRunner(machine),
    handle(MullSimpleTestDummyHandle),
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
  orc::JITSymbol Symbol = ObjectLayer.findSymbol(mangler.getNameWithPrefix(Function.getName()), true);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(Symbol.getAddress()));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

void SimpleTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation) {
  if (handle != MullSimpleTestDummyHandle) {
    ObjectLayer.removeObjectSet(handle);
  }

  handle = ObjectLayer.addObjectSet(objectFiles,
                                    make_unique<SectionMemoryManager>(),
                                    make_unique<InstrumentationResolver>(overrides,
                                                                         instrumentation,
                                                                         mangler,
                                                                         trampoline));
  ObjectLayer.emitAndFinalize(handle);
}

void SimpleTestRunner::loadProgram(ObjectFiles &objectFiles) {
  if (handle != MullSimpleTestDummyHandle) {
    ObjectLayer.removeObjectSet(handle);
  }
  handle = ObjectLayer.addObjectSet(objectFiles,
                                    make_unique<SectionMemoryManager>(),
                                    make_unique<NativeResolver>(overrides));
  ObjectLayer.emitAndFinalize(handle);
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
