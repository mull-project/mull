#include "mull/Mutators/RemoveVoidFunctionMutator.h"

#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Mutators/Mutator.h"

#include <gtest/gtest.h>
#include <irm/irm.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

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
    auto points = mutator.getMutations(bitcode.get(), &function);
    std::copy(points.begin(), points.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}
