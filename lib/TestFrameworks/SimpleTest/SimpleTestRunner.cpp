#include "mull/TestFrameworks/SimpleTest/SimpleTestRunner.h"

#include "mull/TestFrameworks/Test.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Resolvers/InstrumentationResolver.h"
#include "mull/Toolchain/Resolvers/MutationResolver.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Function.h>

#include <string>

using namespace mull;
using namespace llvm;

SimpleTestRunner::SimpleTestRunner(Mangler &mangler)
    : mangler(mangler), overrides([this](const char *name) {
        return this->mangler.getNameWithPrefix(name);
      }),
      trampoline(new InstrumentationInfo *) {}

SimpleTestRunner::~SimpleTestRunner() { delete trampoline; }

void *SimpleTestRunner::functionPointer(const std::string &functionName,
                                        JITEngine &jit) {
  auto mangledName = mangler.getNameWithPrefix(functionName);
  llvm_compat::JITSymbol &symbol = jit.getSymbol(mangledName);
  auto address = llvm_compat::JITSymbolAddress(symbol);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(address));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

void SimpleTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation,
                                               JITEngine &jit) {
  InstrumentationResolver resolver(overrides, instrumentation, mangler,
                                   trampoline);
  jit.addObjectFiles(objectFiles, resolver,
                     make_unique<SectionMemoryManager>());
}

void SimpleTestRunner::loadMutatedProgram(ObjectFiles &objectFiles,
                                          Trampolines &trampolines,
                                          JITEngine &jit) {
  trampolines.allocateTrampolines(mangler);
  MutationResolver resolver(overrides, trampolines, mangler);
  jit.addObjectFiles(objectFiles, resolver,
                     make_unique<SectionMemoryManager>());
}

ExecutionStatus SimpleTestRunner::runTest(JITEngine &jit, Program &program,
                                          Test &test) {
  *trampoline = &test.getInstrumentationInfo();

  std::vector<std::string> arguments = test.getArguments();
  arguments.insert(arguments.begin(), test.getProgramName());
  size_t argc = arguments.size();
  char **argv = new char *[argc + 1];

  for (int i = 0; i < argc; i++) {
    std::string &argument = arguments[i];
    argv[i] = new char[argument.length() + 1];
    strcpy(argv[i], argument.c_str());
  }
  argv[argc] = nullptr;

  void *mainPointer = functionPointer(test.getDriverFunctionName(), jit);
  auto main = ((int (*)(int, char **))(intptr_t)mainPointer);
  int exitStatus = main(static_cast<int>(argc), argv);

  for (int i = 0; i < argc; i++) {
    delete[] argv[i];
  }
  delete[] argv;

  overrides.runDestructors();

  if (exitStatus == 1) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
