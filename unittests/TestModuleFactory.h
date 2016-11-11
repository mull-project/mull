#include "llvm/IR/Module.h"

#include <string>

using namespace llvm;

class TestModuleFactory {

public:
  std::string testerModulePath_IR();
  std::string testerModulePath_Bitcode();
  std::unique_ptr<Module> createTesterModule();

  std::unique_ptr<Module> createTesteeModule();

  std::unique_ptr<Module> create_SimpleTest_NegateCondition_Tester_Module();
  std::unique_ptr<Module> create_SimpleTest_NegateCondition_Testee_Module();

  std::unique_ptr<Module> createLibCTesterModule();
  std::unique_ptr<Module> createLibCTesteeModule();

  std::unique_ptr<Module> createExternalLibTesterModule();
  std::unique_ptr<Module> createExternalLibTesteeModule();

  std::unique_ptr<Module> createGoogleTestTesterModule();
  std::unique_ptr<Module> createGoogleTestTesteeModule();
};
