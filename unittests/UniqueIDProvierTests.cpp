#include "UniqueIDProvider.h"

#include "MutationOperators/AddMutationOperator.h"
#include "ModuleLoader.h"
#include "MutationPoint.h"

#include "TestModuleFactory.h"
#include "llvm/IR/Module.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;
using namespace std;

static TestModuleFactory testModuleFactory;

TEST(UniqueIDProvider, uniqueIDForModule) {
  LLVMContext context;
  ModuleLoader loader(context);
  auto module = loader.loadModuleAtPath(testModuleFactory.testerModulePath_Bitcode());

  string moduleName = "fixture_simple_test_tester_module";
  string moduleMD5  = "de5070f8606cc2a8ee794b2ab56b31f2";
  string uniqueID   = moduleName + "_" + moduleMD5;

  UniqueIDProvider provider;
  ASSERT_EQ(provider.uniqueIDForModule(*module.get()), uniqueID);
}

TEST(UniqueIDProvider, uniqueIDForModuleWithMutationPoint) {
  LLVMContext context;
  ModuleLoader loader(context);
  auto module = loader.loadModuleAtPath(testModuleFactory.testerModulePath_Bitcode());

  MutationPointAddress address(2, 3, 5);
  AddMutationOperator mutationOperator;

  MutationPoint point(&mutationOperator, address, nullptr);

  string moduleName = "fixture_simple_test_tester_module";
  string moduleMD5  = "de5070f8606cc2a8ee794b2ab56b31f2";
  string addressString = "2_3_5";
  string operatorName = "add";

  string uniqueID = moduleName + "_"
      + moduleMD5 + "_"
      + addressString + "_"
      + operatorName;

  UniqueIDProvider provider;
  ASSERT_EQ(provider.uniqueIDForModuleWithMutationPoint(*module.get(), point), uniqueID);
}
