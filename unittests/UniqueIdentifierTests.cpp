#include "Mutators/MathAddMutator.h"
#include "ModuleLoader.h"
#include "MutationPoint.h"

#include "TestModuleFactory.h"
#include "llvm/IR/Module.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;
using namespace std;

static TestModuleFactory testModuleFactory;

TEST(MullModule, uniqueIdentifier) {
  LLVMContext context;
  ModuleLoader loader(context);
  auto module = loader.loadModuleAtPath(testModuleFactory.testerModulePath_Bitcode());

  string moduleName = "fixture_simple_test_tester_module";
  string moduleMD5  = "de5070f8606cc2a8ee794b2ab56b31f2";
  string uniqueID   = moduleName + "_" + moduleMD5;

  ASSERT_EQ(module->getUniqueIdentifier(), uniqueID);
}

TEST(MutationPoint, uniqueIdentifier) {
  LLVMContext context;
  ModuleLoader loader(context);
  auto module = loader.loadModuleAtPath(testModuleFactory.testerModulePath_Bitcode());

  MutationPointAddress address(2, 3, 5);
  MathAddMutator mutator;

  MutationPoint point(&mutator, address, nullptr, module.get());

  string moduleName = "fixture_simple_test_tester_module";
  string moduleMD5  = "de5070f8606cc2a8ee794b2ab56b31f2";
  string addressString = "2_3_5";
  string mutatorName = "math_add_mutator";

  string uniqueID = moduleName + "_"
      + moduleMD5 + "_"
      + addressString + "_"
      + mutatorName;

  ASSERT_EQ(point.getUniqueIdentifier(), uniqueID);
}
