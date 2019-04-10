#include "mull/TestFrameworks/CustomTestFramework/CustomTestRunner.h"

#include "mull/Program/Program.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTest_Test.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Resolvers/InstrumentationResolver.h"
#include "mull/Toolchain/Resolvers/MutationResolver.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Function.h>

using namespace mull;
using namespace llvm;
using namespace llvm::orc;

CustomTestRunner::CustomTestRunner(Mangler &mangler)
    : mangler(mangler), overrides([this](const char *name) {
        return this->mangler.getNameWithPrefix(name);
      }),
      trampoline(new InstrumentationInfo *) {}

CustomTestRunner::~CustomTestRunner() { delete trampoline; }

void *CustomTestRunner::getConstructorPointer(const llvm::Function &function,
                                              JITEngine &jit) {
  return getFunctionPointer(mangler.getNameWithPrefix(function.getName()), jit);
}

void *CustomTestRunner::getFunctionPointer(const std::string &functionName,
                                           JITEngine &jit) {
  JITSymbol &symbol = jit.getSymbol(functionName);
  auto address = llvm_compat::JITSymbolAddress(symbol);

  void *fpointer = reinterpret_cast<void *>(static_cast<uintptr_t>(address));

  if (fpointer == nullptr) {
    errs() << "CustomTestRunner> Can't find pointer to function: "
           << functionName << "\n";
    exit(1);
  }

  return fpointer;
}

void CustomTestRunner::runStaticConstructor(llvm::Function *function,
                                            JITEngine &jit) {
  //  printf("Init: %s\n", Ctor->getName().str().c_str());

  void *CtorPointer = getConstructorPointer(*function, jit);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

void CustomTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation,
                                               JITEngine &jit) {
  InstrumentationResolver resolver(overrides, instrumentation, mangler,
                                   trampoline);
  jit.addObjectFiles(objectFiles, resolver,
                     make_unique<SectionMemoryManager>());
}

void CustomTestRunner::loadMutatedProgram(ObjectFiles &objectFiles,
                                          Trampolines &trampolines,
                                          JITEngine &jit) {
  trampolines.allocateTrampolines(mangler);
  MutationResolver resolver(overrides, trampolines, mangler);
  jit.addObjectFiles(objectFiles, resolver,
                     make_unique<SectionMemoryManager>());
}

ExecutionStatus CustomTestRunner::runTest(JITEngine &jit, Program &program,
                                          Test *test) {
  *trampoline = &test->getInstrumentationInfo();

  CustomTest_Test *customTest = dyn_cast<CustomTest_Test>(test);

  for (auto &constructor : program.getStaticConstructors()) {
    runStaticConstructor(constructor, jit);
  }

  std::vector<std::string> arguments = customTest->getArguments();
  arguments.insert(arguments.begin(), customTest->getProgramName());
  int argc = arguments.size();
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
  int exitStatus = main(argc, argv);

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
