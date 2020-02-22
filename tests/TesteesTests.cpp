#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/ReachableFunction.h"
#include "gtest/gtest.h"
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;

TEST(FunctionsUnderTest, mergeReachableFunctions) {
  Diagnostics diagnostics;
  llvm::LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_and_or_and_to_or_replacement_module_bc_path(), llvmContext, diagnostics);

  auto &allFunctions = bitcode->getModule()->getFunctionList();

  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  for (auto &func : allFunctions) {
    reachableFunctions.push_back(llvm::make_unique<ReachableFunction>(&func, nullptr, 1));
    reachableFunctions.push_back(llvm::make_unique<ReachableFunction>(&func, nullptr, 2));
    reachableFunctions.push_back(llvm::make_unique<ReachableFunction>(&func, nullptr, 3));
  }

  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);

  ASSERT_EQ(allFunctions.size(), functionsUnderTest.size());

  for (auto &merged : functionsUnderTest) {
    ASSERT_EQ(size_t(3), merged.getReachableTests().size());
  }
}
