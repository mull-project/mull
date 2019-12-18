#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <mull/BitcodeLoader.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/Mutators/CXX/RelationalMutators.h>
#include <mull/ReachableFunction.h>

using namespace mull;
using namespace llvm;

TEST(Relational, LessThanToLessOrEqual) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode =
      loader.loadBitcodeAtPath(fixtures::mutators_boundary_module_bc_path(), context, diagnostics);

  cxx::LessThanToLessOrEqual mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(3));
}

TEST(Relational, LessOrEqualToLessThan) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode =
      loader.loadBitcodeAtPath(fixtures::mutators_boundary_module_bc_path(), context, diagnostics);

  cxx::LessOrEqualToLessThan mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}

TEST(Relational, GreaterThanToGreaterOrEqual) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode =
      loader.loadBitcodeAtPath(fixtures::mutators_boundary_module_bc_path(), context, diagnostics);

  cxx::GreaterThanToGreaterOrEqual mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(2));
}

TEST(Relational, GreaterOrEqualToGreaterThan) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode =
      loader.loadBitcodeAtPath(fixtures::mutators_boundary_module_bc_path(), context, diagnostics);

  cxx::GreaterOrEqualToGreaterThan mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}
