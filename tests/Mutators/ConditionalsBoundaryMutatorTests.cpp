#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <mull/BitcodeLoader.h>
#include <mull/Mutators/CXX/RelationalMutators.h>

using namespace mull;
using namespace llvm;

TEST(Relational, LessThanToLessOrEqual) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), context);

  cxx::LessThanToLessOrEqual mutator;
  std::vector<MutationPoint *> mutants;
  NullInstructionFilter instructionFilter;
  for (auto &function : bitcode->getModule()->functions()) {
    auto m = mutator.getMutations(bitcode.get(), &function, instructionFilter);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(3));
}

TEST(Relational, LessOrEqualToLessThan) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), context);

  cxx::LessOrEqualToLessThan mutator;
  std::vector<MutationPoint *> mutants;
  NullInstructionFilter instructionFilter;
  for (auto &function : bitcode->getModule()->functions()) {
    auto m = mutator.getMutations(bitcode.get(), &function, instructionFilter);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}

TEST(Relational, GreaterThanToGreaterOrEqual) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), context);

  cxx::GreaterThanToGreaterOrEqual mutator;
  std::vector<MutationPoint *> mutants;
  NullInstructionFilter instructionFilter;
  for (auto &function : bitcode->getModule()->functions()) {
    auto m = mutator.getMutations(bitcode.get(), &function, instructionFilter);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(2));
}

TEST(Relational, GreaterOrEqualToGreaterThan) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), context);

  cxx::GreaterOrEqualToGreaterThan mutator;
  std::vector<MutationPoint *> mutants;
  NullInstructionFilter instructionFilter;
  for (auto &function : bitcode->getModule()->functions()) {
    auto m = mutator.getMutations(bitcode.get(), &function, instructionFilter);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}
