#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Testee.h"
#include "gtest/gtest.h"

using namespace mull;

TEST(Testees, mergeTestees) {
  llvm::LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_and_or_replacement_module_bc_path(), llvmContext);

  auto &allFunctions = bitcode->getModule()->getFunctionList();

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
