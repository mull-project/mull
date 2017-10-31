#include "SimpleTest/SimpleTestRunner.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"

/// TODO: enable back for LLVM 4.0
//#include "llvm/ExecutionEngine/JITSymbol.h"
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

class Mull_SimpleTest_Resolver : public RuntimeDyld::SymbolResolver {
public:

  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) {
    //if (Name == "_printf") {
    //  return findSymbol("mull_simple_test_printf");
    //}

    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);
    }

    return RuntimeDyld::SymbolInfo(nullptr);
  }

  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &Name) {
    return RuntimeDyld::SymbolInfo(nullptr);
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
  orc::JITSymbol Symbol = ObjectLayer.findSymbol(MangleName(Function.getName()), true);
  void *FPointer = reinterpret_cast<void *>(static_cast<uintptr_t>(Symbol.getAddress()));
  assert(FPointer && "Can't find pointer to function");
  return FPointer;
}

ExecutionStatus SimpleTestRunner::runTest(Test *test, ObjectFiles &objectFiles) {
  assert(isa<SimpleTest_Test>(test) && "Supposed to work only with");

  SimpleTest_Test *SimpleTest = dyn_cast<SimpleTest_Test>(test);

  auto Handle = ObjectLayer.addObjectSet(objectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<Mull_SimpleTest_Resolver>());
  void *FunctionPointer = TestFunctionPointer(*SimpleTest->GetTestFunction());

  uint64_t result = ((int (*)())(intptr_t)FunctionPointer)();

  ObjectLayer.removeObjectSet(Handle);

  if (result == 1) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
