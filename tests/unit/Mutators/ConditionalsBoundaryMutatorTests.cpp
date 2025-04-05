#include "tests/unit/FixturePaths.h"
#include "tests/unit/helpers/BitcodeLoader.h"
#include <gtest/gtest.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/FunctionUnderTest.h>
#include <mull/Mutators/CXX/RelationalMutators.h>

using namespace mull;
using namespace llvm;

TEST(Relational, LessThanToLessOrEqual) {
  Diagnostics diagnostics;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::tests_unit_fixtures_mutators_boundary_module_cpp_bc_path(), diagnostics);

  cxx::LessThanToLessOrEqual mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, bitcode.get());
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(3));
}

TEST(Relational, LessOrEqualToLessThan) {
  Diagnostics diagnostics;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::tests_unit_fixtures_mutators_boundary_module_cpp_bc_path(), diagnostics);

  cxx::LessOrEqualToLessThan mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, bitcode.get());
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}

TEST(Relational, GreaterThanToGreaterOrEqual) {
  Diagnostics diagnostics;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::tests_unit_fixtures_mutators_boundary_module_cpp_bc_path(), diagnostics);

  cxx::GreaterThanToGreaterOrEqual mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, bitcode.get());
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(2));
}

TEST(Relational, GreaterOrEqualToGreaterThan) {
  Diagnostics diagnostics;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::tests_unit_fixtures_mutators_boundary_module_cpp_bc_path(), diagnostics);

  cxx::GreaterOrEqualToGreaterThan mutator;
  std::vector<MutationPoint *> mutants;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, bitcode.get());
    functionUnderTest.selectInstructions({});
    auto m = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(m.begin(), m.end(), std::back_inserter(mutants));
  }

  ASSERT_EQ(mutants.size(), size_t(1));
}
