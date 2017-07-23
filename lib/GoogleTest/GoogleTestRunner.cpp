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
#include <queue>

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

extern "C" int mull__cxa_atexit(mull_destructor_t destructor, void *arg, void *__dso_handle) {
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

extern "C" void *mull__dso_handle = nullptr;

//class Mull_GoogleTest_Resolver : public RuntimeDyld::SymbolResolver {
//public:
//
//  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) {
//    if (Name == "___cxa_atexit") {
//      return findSymbol("mull__cxa_atexit");
//    }
//
//    if (Name == "___dso_handle") {
//      return findSymbol("mull__dso_handle");
//    }
//
//    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
//      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);
//
//    return RuntimeDyld::SymbolInfo(nullptr);
//  }
//
//  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &Name) {
//    return RuntimeDyld::SymbolInfo(nullptr);
//  }
//};

GoogleTestRunner::GoogleTestRunner(llvm::TargetMachine &machine)
  : TestRunner(machine), jit(machine) {}

std::string GoogleTestRunner::MangleName(const llvm::StringRef &Name) {
  std::string MangledName;
  {
    raw_string_ostream Stream(MangledName);
    Mangler.getNameWithPrefix(Stream, Name, machine.createDataLayout());
  }
  return MangledName;
}

void *GoogleTestRunner::GetCtorPointer(const llvm::Function &Function) {
  return FunctionPointer(MangleName(Function.getName()).c_str());
}

void *GoogleTestRunner::FunctionPointer(const char *functionName) {
  JITSymbol Symbol = jit.jit().findSymbol(functionName, false);
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

ExecutionResult GoogleTestRunner::runTest(Test *Test, std::vector<llvm::Module *> &modules) {
  GoogleTest_Test *GTest = dyn_cast<GoogleTest_Test>(Test);
  errs() << GTest->GetTestBodyFunction()->getName();

  typedef std::function<RuntimeDyld::SymbolInfo (const std::string&)> resolver_t;

  resolver_t localLookup = [&] (const std::string &Name)
  {
    if (auto Sym = jit.jit().findSymbol(Name, false))
      return Sym.toRuntimeDyldSymbol();
    return RuntimeDyld::SymbolInfo(nullptr);
  };

  /// Recursive labmda! Yay!
  resolver_t externalLookup = [&](const std::string &Name) {
    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);
    }

    if (Name == "___cxa_atexit") {
      return externalLookup("mull__cxa_atexit");
    }

    if (Name == "___dso_handle") {
      return externalLookup("mull__dso_handle");
    }

    return RuntimeDyld::SymbolInfo(nullptr);
  };

  auto resolver = createLambdaResolver(localLookup, externalLookup);

  jit.jit().addModuleSet(modules,
                         make_unique<SectionMemoryManager>(),
                         std::move(resolver));

  jit.jit().prepareForExecution();

  auto start = high_resolution_clock::now();

  for (auto &Ctor: GTest->GetGlobalCtors()) {
    runStaticCtor(Ctor);
  }

  std::string filter = "--gtest_filter=" + GTest->getTestName();
  const char *argv[] = { "mull", filter.c_str(), NULL };
  int argc = 2;

  /// Normally Google Test Driver looks like this:
  ///
  ///   int main(int argc, char **argv) {
  ///     InitGoogleTest(&argc, argv);
  ///     return UnitTest.GetInstance()->Run();
  ///   }
  ///
  /// Technically we can just call `main` function, but there is a problem:
  /// Among all the files that are being processed may be more than one
  /// `main` function, therefore we can call wrong driver.
  ///
  /// To avoid this from happening we implement the driver function on our own.
  /// We must keep in mind that each project can have its own, extended
  /// version of the driver (LLVM itself has one).
  ///

  void *initGTestPtr = FunctionPointer("__ZN7testing14InitGoogleTestEPiPPc");
  auto initGTest = ((void (*)(int *, const char**))(intptr_t)initGTestPtr);
  initGTest(&argc, argv);

  void *getInstancePtr = FunctionPointer("__ZN7testing8UnitTest11GetInstanceEv");
  auto getInstance = ((UnitTest *(*)())(intptr_t)getInstancePtr);
  UnitTest *test = getInstance();

  void *runAllTestsPtr = FunctionPointer("__ZN7testing8UnitTest3RunEv");
  auto runAllTests = ((int (*)(UnitTest *))(intptr_t)runAllTestsPtr);
  uint64_t result = runAllTests(test);

  runDestructors();
  auto elapsed = high_resolution_clock::now() - start;

  //printf("%llu %s\n", result, GTest->getTestName().c_str());

  ExecutionResult Result;
  Result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

  std::unique_ptr<CallTree> callTree(jit.jit().createCallTree());
  std::queue<CallTree *> tree;
  tree.push(callTree.get());
  while (!tree.empty()) {
    CallTree *node = tree.front();
    assert(node);
    tree.pop();
    for (auto &child: node->children) {
      assert(child->function);
      errs() << child->level << " " << child->function->getName() << "\n";
      tree.push(child.get());
    }
  }

  if (result == 0) {
    Result.Status = ExecutionStatus::Passed;
  } else {
    Result.Status = ExecutionStatus::Failed;
  }
  
  return Result;
}

ExecutionResult GoogleTestRunner::runTest(Test *Test, ObjectFiles &ObjectFiles) {
  GoogleTest_Test *GTest = dyn_cast<GoogleTest_Test>(Test);

//  auto Handle = ObjectLayer.addObjectSet(ObjectFiles,
//                                         make_unique<SectionMemoryManager>(),
//                                         make_unique<Mull_GoogleTest_Resolver>());

  auto start = high_resolution_clock::now();

  for (auto &Ctor: GTest->GetGlobalCtors()) {
    runStaticCtor(Ctor);
  }

  std::string filter = "--gtest_filter=" + GTest->getTestName();
  const char *argv[] = { "mull", filter.c_str(), NULL };
  int argc = 2;

  /// Normally Google Test Driver looks like this:
  ///
  ///   int main(int argc, char **argv) {
  ///     InitGoogleTest(&argc, argv);
  ///     return UnitTest.GetInstance()->Run();
  ///   }
  ///
  /// Technically we can just call `main` function, but there is a problem:
  /// Among all the files that are being processed may be more than one
  /// `main` function, therefore we can call wrong driver.
  ///
  /// To avoid this from happening we implement the driver function on our own.
  /// We must keep in mind that each project can have its own, extended
  /// version of the driver (LLVM itself has one).
  ///

  void *initGTestPtr = FunctionPointer("__ZN7testing14InitGoogleTestEPiPPc");
  auto initGTest = ((void (*)(int *, const char**))(intptr_t)initGTestPtr);
  initGTest(&argc, argv);

  void *getInstancePtr = FunctionPointer("__ZN7testing8UnitTest11GetInstanceEv");
  auto getInstance = ((UnitTest *(*)())(intptr_t)getInstancePtr);
  UnitTest *test = getInstance();

  void *runAllTestsPtr = FunctionPointer("__ZN7testing8UnitTest3RunEv");
  auto runAllTests = ((int (*)(UnitTest *))(intptr_t)runAllTestsPtr);
  uint64_t result = runAllTests(test);

  runDestructors();
  auto elapsed = high_resolution_clock::now() - start;

  //printf("%llu %s\n", result, GTest->getTestName().c_str());

  ExecutionResult Result;
  Result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

//  ObjectLayer.removeObjectSet(Handle);

  if (result == 0) {
    Result.Status = ExecutionStatus::Passed;
  } else {
    Result.Status = ExecutionStatus::Failed;
  }

  return Result;
}
