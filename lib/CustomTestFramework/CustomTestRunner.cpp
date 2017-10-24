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
public:

  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) {
    if (Name == "___cxa_atexit") {
      return findSymbol("mull_custom_test_cxa_atexit");
    }

    if (Name == "___dso_handle") {
      return findSymbol("mull_custom_test_dso_handle");
    }

    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);

    return RuntimeDyld::SymbolInfo(nullptr);
  }

  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &Name) {
    return RuntimeDyld::SymbolInfo(nullptr);   }
};

CustomTestRunner::CustomTestRunner(llvm::TargetMachine &machine)
  : TestRunner(machine) {}

std::string CustomTestRunner::MangleName(const llvm::StringRef &Name) {
  std::string MangledName;
  {
    raw_string_ostream Stream(MangledName);
    Mangler.getNameWithPrefix(Stream, Name, machine.createDataLayout());
  }
  return MangledName;
}

void *CustomTestRunner::GetCtorPointer(const llvm::Function &Function) {
  return FunctionPointer(MangleName(Function.getName()).c_str());
}

void *CustomTestRunner::FunctionPointer(const char *FunctionName) {
  JITSymbol Symbol = ObjectLayer.findSymbol(FunctionName, false);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(Symbol.getAddress()));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

void CustomTestRunner::runStaticCtor(llvm::Function *Ctor) {
//  printf("Init: %s\n", Ctor->getName().str().c_str());

  void *CtorPointer = GetCtorPointer(*Ctor);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

ExecutionResult CustomTestRunner::runTest(Test *test, ObjectFiles &objectFiles) {
  CustomTest_Test *customTest = dyn_cast<CustomTest_Test>(test);

  auto Handle = ObjectLayer.addObjectSet(objectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<Mull_CustomTest_Resolver>());

  auto start = high_resolution_clock::now();

  for (auto &constructor: customTest->getConstructors()) {
    runStaticCtor(constructor);
  }

  std::vector<std::string> arguments = customTest->getArguments();
  arguments.insert(arguments.begin(), customTest->getProgramName());
  int argc = arguments.size();
  char **argv = new char*[argc];

  for (int i = 0; i < argc; i++) {
    std::string &argument = arguments[i];
    argv[i] = new char[argument.length()];
    strcpy(argv[i], argument.c_str());
  }

  void *mainPointer = FunctionPointer("_main");
  auto main = ((int (*)(int, char**))(intptr_t)mainPointer);
  int exitStatus = main(argc, argv);

  for (int i = 0; i < argc; i++) {
    delete[] argv[i];
  }
  delete[] argv;

  runDestructors();
  auto elapsed = high_resolution_clock::now() - start;

  ExecutionResult executionResult;
  executionResult.exitStatus = exitStatus;
  executionResult.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

  ObjectLayer.removeObjectSet(Handle);
  if (exitStatus == 0) {
    executionResult.Status = ExecutionStatus::Passed;
  } else {
    executionResult.Status = ExecutionStatus::Failed;
  }

  return executionResult;
}