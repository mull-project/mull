#include "GoogleTest/GoogleTestRunner.h"

#include "GoogleTest/GoogleTest_Test.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/TargetSelect.h"

#include <chrono>
#include <execinfo.h>

using namespace Mutang;
using namespace llvm;
using namespace std::chrono;

typedef void (*mutang_destructor_t)(void *);

struct atexit_entry {
  mutang_destructor_t destructor;
  void *arg;
  void *dso_handle;
};

const static int dtors_count = 64;
static int current_dtor = 0;
static atexit_entry dtors[dtors_count];

extern "C" int mutang__cxa_atexit(mutang_destructor_t destructor, void *arg, void *__dso_handle) {
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

void runDestructors() {
//  printf("dtors: %d\n", current_dtor);
  while (current_dtor > 0) {
    current_dtor--;
//    printf("cleaning dtor: %d: %p\n", current_dtor, dtors[current_dtor].arg);
    dtors[current_dtor].destructor(dtors[current_dtor].arg);
  }
}

/// Hijacking output functions to prevent extra logging

extern "C" int mutang_vprintf(const char *restrict, va_list) {
  return 0;
}

extern "C" int mutang_printf(const char *fmt, ...) {
  /// ignoring
  return 0;
}

extern "C" void *mutang__dso_handle = nullptr;

class Mutang_GoogleTest_Resolver : public JITSymbolResolver {
public:

  JITSymbol findSymbol(const std::string &Name) {
    if (Name == "___cxa_atexit") {
      return findSymbol("mutang__cxa_atexit");
    }

    if (Name == "___dso_handle") {
      return findSymbol("mutang__dso_handle");
    }

    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
      return JITSymbol(SymAddr, JITSymbolFlags::Exported);
    return JITSymbol(nullptr);
  }

  JITSymbol findSymbolInLogicalDylib(const std::string &Name) {
    return JITSymbol(nullptr);
  }
};

GoogleTestRunner::GoogleTestRunner() : TM(EngineBuilder().selectTarget(
                                              Triple(), "", "",
                                              SmallVector<std::string, 1>())) {
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  LLVMLinkInOrcMCJITReplacement();
}

std::string GoogleTestRunner::MangleName(const llvm::StringRef &Name) {
  std::string MangledName;
  {
    raw_string_ostream Stream(MangledName);
    Mangler.getNameWithPrefix(Stream, Name, TM->createDataLayout());
  }
  return MangledName;
}

void *GoogleTestRunner::GetCtorPointer(const llvm::Function &Function) {
  return FunctionPointer(MangleName(Function.getName()).c_str());
}

void *GoogleTestRunner::FunctionPointer(const char *FunctionName) {
  JITSymbol Symbol = ObjectLayer.findSymbol(FunctionName, false);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(Symbol.getAddress()));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

void GoogleTestRunner::runStaticCtor(llvm::Function *Ctor) {
//  printf("Init: %s\n", Ctor->getName().str().c_str());

  void *CtorPointer = GetCtorPointer(*Ctor);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

ExecutionResult GoogleTestRunner::runTest(Test *Test, ObjectFiles &ObjectFiles) {
  GoogleTest_Test *GTest = dyn_cast<GoogleTest_Test>(Test);

  auto Handle = ObjectLayer.addObjectSet(ObjectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<Mutang_GoogleTest_Resolver>());

  auto start = high_resolution_clock::now();

  for (auto &Ctor: GTest->GetGlobalCtors()) {
    runStaticCtor(Ctor);
  }

  void *MainPtr = FunctionPointer("_main");
  auto Main = ((int (*)(int, const char**))(intptr_t)MainPtr);

  std::string filter = "--gtest_filter=" + GTest->getTestName();

  const char *argv[] = { "mutang", filter.c_str(), NULL };

  uint64_t result = Main(2, argv);

  runDestructors();
  auto elapsed = high_resolution_clock::now() - start;

  //printf("%llu %s\n", result, GTest->getTestName().c_str());

  ExecutionResult Result;
  Result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

  ObjectLayer.removeObjectSet(Handle);

  if (result == 0) {
    Result.Status = ExecutionStatus::Passed;
  } else {
    Result.Status = ExecutionStatus::Failed;
  }

  return Result;
}
