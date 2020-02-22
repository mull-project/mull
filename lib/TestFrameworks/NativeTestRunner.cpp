#include "mull/TestFrameworks/NativeTestRunner.h"

#include "LLVMCompatibility.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/Test.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Resolvers/InstrumentationResolver.h"
#include "mull/Toolchain/Resolvers/MutationResolver.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/SectionMemoryManager.h>

using namespace mull;

NativeTestRunner::NativeTestRunner(Diagnostics &diagnostics, Mangler &mangler)
    : diagnostics(diagnostics), mangler(mangler), overrides(mangler),
      trampoline(new InstrumentationInfo *) {}

NativeTestRunner::~NativeTestRunner() {
  delete trampoline;
}

void *NativeTestRunner::getConstructorPointer(const llvm::Function &function, JITEngine &jit) {
  auto name = mangler.getNameWithPrefix(function.getName().str());
  return getFunctionPointer(name, jit);
}

void *NativeTestRunner::getFunctionPointer(const std::string &functionName, JITEngine &jit) {
  llvm_compat::JITSymbol &symbol = jit.getSymbol(functionName);
  auto address = llvm_compat::JITSymbolAddress(symbol);

  void *pointer = reinterpret_cast<void *>(static_cast<uintptr_t>(address));

  if (pointer == nullptr) {
    diagnostics.error(std::string("Can not find a pointer to the function: " + functionName));
  }

  return pointer;
}

void NativeTestRunner::runStaticConstructor(llvm::Function *constructor, JITEngine &jit) {
  void *CtorPointer = getConstructorPointer(*constructor, jit);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

void NativeTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation, JITEngine &jit) {
  InstrumentationResolver resolver(overrides, instrumentation, mangler, trampoline);
  jit.addObjectFiles(objectFiles, resolver, llvm::make_unique<llvm::SectionMemoryManager>());
}

ExecutionStatus NativeTestRunner::runTest(JITEngine &jit, Program &program, Test &test) {
  *trampoline = &test.getInstrumentationInfo();

  for (auto &constructor : program.getStaticConstructors()) {
    runStaticConstructor(constructor, jit);
  }

  std::vector<std::string> arguments = test.getArguments();
  arguments.insert(arguments.begin(), test.getProgramName());
  size_t argc = arguments.size();
  char **argv = new char *[argc + 1];

  for (size_t i = 0; i < argc; i++) {
    std::string &argument = arguments[i];
    argv[i] = new char[argument.length() + 1];
    strcpy(argv[i], argument.c_str());
  }
  argv[argc] = nullptr;

  void *mainPointer =
      getFunctionPointer(mangler.getNameWithPrefix(test.getDriverFunctionName()), jit);
  auto main = ((int (*)(int, char **))(intptr_t)mainPointer);
  int exitStatus = main(static_cast<int>(argc), argv);

  for (int i = 0; i < argc; i++) {
    delete[] argv[i];
  }
  delete[] argv;

  overrides.runDestructors();

  if (exitStatus == 0) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}

void NativeTestRunner::loadMutatedProgram(TestRunner::ObjectFiles &objectFiles,
                                          Trampolines &trampolines, JITEngine &jit) {
  trampolines.allocateTrampolines(mangler);
  MutationResolver resolver(overrides, trampolines);
  jit.addObjectFiles(objectFiles, resolver, llvm::make_unique<llvm::SectionMemoryManager>());
}
