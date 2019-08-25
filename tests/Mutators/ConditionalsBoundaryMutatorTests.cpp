#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/MutationPoint.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/ConditionalsBoundaryMutator.h"
#include "mull/Program/Program.h"
#include "mull/Testee.h"
#include "mull/Toolchain/Compiler.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(ConditionalsBoundaryMutator, findMutations) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), context);

  ConditionalsBoundaryMutator mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    auto m = mutator.getMutations(bitcode.get(), &function);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), 7U);
}

TEST(ConditionalsBoundaryMutator, applyMutations) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), context);

  ConditionalsBoundaryMutator mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    auto m = mutator.getMutations(bitcode.get(), &function);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  for (auto mutant : mutants) {
    ValueToValueMapTy map;
    auto mutatedFunction = CloneFunction(mutant->getOriginalFunction(), map);

    Instruction *originalInstruction =
        &mutant->getAddress().findInstruction(mutant->getOriginalFunction());
    mutant->setMutatedFunction(mutatedFunction);
    mutant->applyMutation();
    Instruction *mutatedInstruction =
        &mutant->getAddress().findInstruction(mutatedFunction);

    if (ConditionalsBoundaryMutator::isGT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGTE(mutatedInstruction));
    } else if (ConditionalsBoundaryMutator::isGTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGT(mutatedInstruction));
    } else if (ConditionalsBoundaryMutator::isLTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLT(mutatedInstruction));
    } else if (ConditionalsBoundaryMutator::isLT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLTE(mutatedInstruction));
    }
  }

  ASSERT_EQ(mutants.size(), 7U);
}
