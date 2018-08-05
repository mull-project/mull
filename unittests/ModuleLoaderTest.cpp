
#include "gtest/gtest.h"

#include "ModuleLoader.h"
#include "TestModuleFactory.h"

#include <fstream>
#include <iostream>

using namespace mull;
using namespace llvm;
using namespace std;

static TestModuleFactory testModuleFactory;

TEST(ModuleLoaderTest, loadModuleFromBitcodeListFile) {
  llvm::LLVMContext context;

  ModuleLoader loader;

  std::string bitcodeFile = testModuleFactory.testerModulePath_Bitcode();

  std::vector<std::string> bitcodePaths = { bitcodeFile };
  std::vector<std::unique_ptr<MullModule>> modules =
    loader.loadModulesFromBitcodeFileList(bitcodePaths);

  ASSERT_EQ(modules.size(), 1U);
}
