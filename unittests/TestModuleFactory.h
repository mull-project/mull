#include "llvm/IR/Module.h"

using namespace llvm;

class TestModuleFactory {

public:
  std::unique_ptr<Module> createTesterModule();
  std::unique_ptr<Module> createTesteeModule();

  std::unique_ptr<Module> createLibCTesterModule();
  std::unique_ptr<Module> createLibCTesteeModule();

  std::unique_ptr<Module> createExternalLibTesterModule();
  std::unique_ptr<Module> createExternalLibTesteeModule();

  std::unique_ptr<Module> createGoogleTestTesterModule();
};
