#include "SimpleTest/SimpleTestRunner.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"

#include "SimpleTest/SimpleTest_Test.h"

#include <chrono>
#include <cstdarg>

using namespace mull;
using namespace llvm;
using namespace std::chrono;

extern "C" int mull_simple_test_printf(const char *fmt, ...) {
  va_list arglist;

  printf("Printf called from JIT: \n");

  va_start(arglist, fmt);
  vprintf(fmt, arglist);
  va_end(arglist);

  return 0;
}

class Mull_SimpleTest_Resolver : public JITSymbolResolver {
public:

  JITSymbol findSymbol(const std::string &Name) {
    //if (Name == "_printf") {
    //  return findSymbol("mull_simple_test_printf");
    //}

    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
      return JITSymbol(SymAddr, JITSymbolFlags::Exported);
    }

    return JITSymbol(nullptr);
  }

  JITSymbol findSymbolInLogicalDylib(const std::string &Name) {
    return JITSymbol(nullptr);
  }
};

SimpleTestRunner::SimpleTestRunner(TargetMachine &machine)
  : TestRunner(machine) {}

std::string SimpleTestRunner::MangleName(const llvm::StringRef &Name) {
  std::string MangledName;
  {
    raw_string_ostream Stream(MangledName);
    Mangler.getNameWithPrefix(Stream, Name, machine.createDataLayout());
  }
  return MangledName;
}

void *SimpleTestRunner::TestFunctionPointer(const llvm::Function &Function) {
  JITSymbol Symbol = ObjectLayer.findSymbol(MangleName(Function.getName()), true);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(Symbol.getAddress()));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

ExecutionResult SimpleTestRunner::runTest(Test *Test, ObjectFiles &ObjectFiles) {
  assert(isa<SimpleTest_Test>(Test) && "Supposed to work only with");

  SimpleTest_Test *SimpleTest = dyn_cast<SimpleTest_Test>(Test);

  auto Handle = ObjectLayer.addObjectSet(ObjectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<Mull_SimpleTest_Resolver>());
  void *FunctionPointer = TestFunctionPointer(*SimpleTest->GetTestFunction());

  auto start = high_resolution_clock::now();
  uint64_t result = ((int (*)())(intptr_t)FunctionPointer)();
  auto elapsed = high_resolution_clock::now() - start;

  ExecutionResult Result;
  Result.RunningTime = duration_cast<std::chrono::nanoseconds>(elapsed).count();

  ObjectLayer.removeObjectSet(Handle);

  if (result == 1) {
    Result.Status = ExecutionStatus::Passed;
  } else {
    Result.Status = ExecutionStatus::Failed;
  }

  return Result;
}
