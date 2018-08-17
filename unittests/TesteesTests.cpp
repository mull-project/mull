#include "gtest/gtest.h"
#include "TestModuleFactory.h"
#include "Testee.h"

TEST(Testees, mergeTestees) {
  TestModuleFactory factory;
  auto module = factory.create_SimpleTest_ANDORReplacement_Module();
  auto &allFunctions = module->getModule()->getFunctionList();

  std::vector<std::unique_ptr<Testee>> allTestees;
  for (auto &func : allFunctions) {
    allTestees.push_back(llvm::make_unique<Testee>(&func, nullptr, 1));
    allTestees.push_back(llvm::make_unique<Testee>(&func, nullptr, 2));
    allTestees.push_back(llvm::make_unique<Testee>(&func, nullptr, 3));
  }

  auto mergedTestees = mergeTestees(allTestees);

  ASSERT_EQ(allFunctions.size(), mergedTestees.size());

  for (auto &merged : mergedTestees) {
    ASSERT_EQ(size_t(3), merged.getReachableTests().size());
  }
}
