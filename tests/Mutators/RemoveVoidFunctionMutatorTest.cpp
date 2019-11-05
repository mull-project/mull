#include "mull/Mutators/RemoveVoidFunctionMutator.h"

#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Mutators/Mutator.h"
#include "mull/ReachableFunction.h"

#include <gtest/gtest.h>
#include <llvm/IR/LLVMContext.h>

using namespace mull;
using namespace llvm;

TEST(RemoveVoidFunctionMutator, getMutationPoints) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_remove_void_function_testee_bc_path(), context);

  RemoveVoidFunctionMutator mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto points = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(points.begin(), points.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}
