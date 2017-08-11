#include "XCTest/XCTestRunner.h"

#include "Config.h"
#include "Logger.h"
#include "ForkProcessSandbox.h"
#include "XCTest/XCTest_Test.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/TargetSelect.h"

#include <chrono>
#include <dlfcn.h>
#include <execinfo.h>

using namespace mull;
using namespace llvm;
using namespace std::chrono;

class Mull_XCTest_Resolver : public RuntimeDyld::SymbolResolver {
public:

  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) {
    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
      return RuntimeDyld::SymbolInfo(SymAddr, JITSymbolFlags::Exported);
    }

    return RuntimeDyld::SymbolInfo(nullptr);
  }

  RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &Name) {
    return RuntimeDyld::SymbolInfo(nullptr);
  }
};

XCTestRunner::XCTestRunner(llvm::TargetMachine &machine)
: TestRunner(machine) {
  int core = sys::DynamicLibrary::LoadLibraryPermanently("/opt/swift/usr/lib/swift/linux/libswiftCore.so");
  int onon = sys::DynamicLibrary::LoadLibraryPermanently("/opt/swift/usr/lib/swift/linux/libswiftSwiftOnoneSupport.so");
  int foundation = sys::DynamicLibrary::LoadLibraryPermanently("/opt/swift/usr/lib/swift/linux/libFoundation.so");
  int xctest = sys::DynamicLibrary::LoadLibraryPermanently("/opt/swift/usr/lib/swift/linux/libXCTest.so");
  printf("SwiftCore: %d\nOnone: %d\nFoundation: %d\nxctest: %d\n", core, onon, foundation, xctest);
}

void *XCTestRunner::FunctionPointer(const char *functionName) {
  orc::JITSymbol symbol = objectLayer.findSymbol(functionName, false);

  void *FPointer =
    reinterpret_cast<void *>(static_cast<uintptr_t>(symbol.getAddress()));

  printf("getting function %s\n", functionName);

  assert(FPointer && "Can't find pointer to function");

  return FPointer;
}

struct SSS {
	const char *ptr;
};

ExecutionResult XCTestRunner::runTest(Test *Test, ObjectFiles &objectFiles) {
  XCTest_Test *concreteTest = dyn_cast<XCTest_Test>(Test);

  auto handle = objectLayer.addObjectSet(objectFiles,
                                         make_unique<SectionMemoryManager>(),
                                         make_unique<Mull_XCTest_Resolver>());

  auto start = high_resolution_clock::now();

  void *mainPointer = FunctionPointer("main");

  auto main = ((int (*)(int, const char **))(intptr_t)mainPointer);
  const int argc = 2;
  auto testName = concreteTest->getTestName();
  testName += '\0';
  const char *testName_c = testName.c_str();
  printf("%s:%d\n", testName_c, strlen(testName_c));
  const char *argv[] = { "mull", testName_c, NULL };
  auto res = main(argc, argv);

  auto elapsed = high_resolution_clock::now() - start;

  ExecutionResult Result;
  Result.Status = Passed;
  Result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

  objectLayer.removeObjectSet(handle);

  return Result;
}
