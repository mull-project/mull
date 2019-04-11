#include "mull/TestFrameworks/GoogleTest/GoogleTestRunner.h"

#include "mull/Program/Program.h"
#include "mull/TestFrameworks/Test.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Resolvers/InstrumentationResolver.h"
#include "mull/Toolchain/Resolvers/MutationResolver.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Function.h>

using namespace mull;
using namespace llvm;
using namespace llvm::orc;

GoogleTestRunner::GoogleTestRunner(Mangler &mangler)
    : mangler(mangler), overrides([this](const char *name) {
        return this->mangler.getNameWithPrefix(name);
      }),
      trampoline(new InstrumentationInfo *) {}

GoogleTestRunner::~GoogleTestRunner() { delete trampoline; }

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
  InstrumentationResolver resolver(overrides, instrumentation, mangler,
                                   trampoline);
  jit.addObjectFiles(objectFiles, resolver,
                     make_unique<SectionMemoryManager>());
}

ExecutionStatus GoogleTestRunner::runTest(JITEngine &jit, Program &program,
                                          Test &test) {
  *trampoline = &test.getInstrumentationInfo();

  for (auto &constructor : program.getStaticConstructors()) {
    runStaticConstructor(constructor, jit);
  }

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

  void *mainPointer =
      getFunctionPointer(mangler.getNameWithPrefix("main"), jit);
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

void GoogleTestRunner::loadMutatedProgram(TestRunner::ObjectFiles &objectFiles,
                                          Trampolines &trampolines,
                                          JITEngine &jit) {
  trampolines.allocateTrampolines(mangler);
  MutationResolver resolver(overrides, trampolines, mangler);
  jit.addObjectFiles(objectFiles, resolver,
                     make_unique<SectionMemoryManager>());
}
