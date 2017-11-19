#include "CustomTestFramework/CustomTestRunner.h"
#include "CustomTestFramework/CustomTest_Test.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/OrcMCJITReplacement.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>

#include <chrono>
#include <execinfo.h>

using namespace mull;
using namespace llvm;
using namespace std::chrono;
using namespace llvm::orc;

namespace {
  class UnitTest;
}

typedef void (*mull_destructor_t)(void *);

struct atexit_entry {
  mull_destructor_t destructor;
  void *arg;
  void *dso_handle;
};

const static int dtors_count = 64;
static int current_dtor = 0;
static atexit_entry dtors[dtors_count];

extern "C" int mull_custom_test_cxa_atexit(mull_destructor_t destructor, void *arg, void *__dso_handle) {
  assert(current_dtor < dtors_count);

#if 0
  void* callstack[128];
  int i, frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
  for (i = 0; i < frames; ++i) {
    printf("%s\n", strs[i]);
  }
  free(strs);
#endif

  for (int i = 0; i < current_dtor; i++) {
    if (arg == dtors[i].arg) {
//      printf("dtor already registered: %d: %p\n", i, arg);
      return 0;
    }
  }

//  printf("record dtor: %d: %p\n", current_dtor, arg);

  dtors[current_dtor].destructor = destructor;
  dtors[current_dtor].arg = arg;
  dtors[current_dtor].dso_handle = __dso_handle;

  current_dtor++;

  return 0;
}

static void runDestructors() {
//  printf("dtors: %d\n", current_dtor);
  while (current_dtor > 0) {
    current_dtor--;
//    printf("cleaning dtor: %d: %p\n", current_dtor, dtors[current_dtor].arg);
    dtors[current_dtor].destructor(dtors[current_dtor].arg);
  }
}

extern "C" void *mull_custom_test_dso_handle = nullptr;

class Mull_CustomTest_Resolver : public RuntimeDyld::SymbolResolver {
  std::map<std::string, std::string> mapping;

public:

  Mull_CustomTest_Resolver(std::map<std::string, std::string> mapping)
    : mapping(mapping) {}

  RuntimeDyld::SymbolInfo findSymbol(const std::string &name) {
    if (mapping.count(name) > 0) {
      return findSymbol(mapping[name]);
    }

    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);
    }

    return RuntimeDyld::SymbolInfo(nullptr);
  }

  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &Name) {
    return RuntimeDyld::SymbolInfo(nullptr);   }
};

CustomTestRunner::CustomTestRunner(llvm::TargetMachine &machine)
  : TestRunner(machine), mangler(Mangler()) {

  // TODO: Would be great to not have all of the following here.
  // Some builder class?
  DataLayout dataLayout = machine.createDataLayout();

  std::string atExitFunction = mangler.getNameWithPrefix("__cxa_atexit",
                                                         dataLayout);
  std::string dsoHandleFunction = mangler.getNameWithPrefix("__dso_handle",
                                                            dataLayout);

  mapping[atExitFunction]    = "mull__cxa_atexit";
  mapping[dsoHandleFunction] = "mull__dso_handle";
  this->mapping = mapping;
}

void *CustomTestRunner::GetCtorPointer(const llvm::Function &Function) {
  return
    getFunctionPointer(mangler.getNameWithPrefix(Function.getName(),
                                                 machine.createDataLayout()));
}

void *CustomTestRunner::getFunctionPointer(const std::string &functionName) {
  JITSymbol symbol = ObjectLayer.findSymbol(functionName, false);

  void *fpointer =
    reinterpret_cast<void *>(static_cast<uintptr_t>(symbol.getAddress()));

  if (fpointer == nullptr) {
    errs() << "CustomTestRunner> Can't find pointer to function: "
    << functionName << "\n";
    exit(1);
  }

  return fpointer;
}

void CustomTestRunner::runStaticCtor(llvm::Function *Ctor) {
//  printf("Init: %s\n", Ctor->getName().str().c_str());

  void *CtorPointer = GetCtorPointer(*Ctor);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

ExecutionStatus CustomTestRunner::runTest(Test *test, ObjectFiles &objectFiles) {
  CustomTest_Test *customTest = dyn_cast<CustomTest_Test>(test);

  auto Handle =
    ObjectLayer.addObjectSet(objectFiles,
                             make_unique<SectionMemoryManager>(),
                             make_unique<Mull_CustomTest_Resolver>(this->mapping));

  for (auto &constructor: customTest->getConstructors()) {
    runStaticCtor(constructor);
  }

  std::vector<std::string> arguments = customTest->getArguments();
  arguments.insert(arguments.begin(), customTest->getProgramName());
  int argc = arguments.size();
  char **argv = new char*[argc];

  for (int i = 0; i < argc; i++) {
    std::string &argument = arguments[i];
    argv[i] = new char[argument.length() + 1];
    strcpy(argv[i], argument.c_str());
  }

  void *mainPointer = getFunctionPointer("_main");
  auto main = ((int (*)(int, char**))(intptr_t)mainPointer);
  int exitStatus = main(argc, argv);

  for (int i = 0; i < argc; i++) {
    delete[] argv[i];
  }
  delete[] argv;

  runDestructors();

  ObjectLayer.removeObjectSet(Handle);

  if (exitStatus == 0) {
    return ExecutionStatus::Passed;
  }

  return ExecutionStatus::Failed;
}
