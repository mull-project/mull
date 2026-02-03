#include "FixturePaths.h"
#include "rust/mull-core/core.rs.h"
#include "tests/unit/Helpers/BitcodeLoader.h"
#include <gtest/gtest.h>
#include <mull/FunctionUnderTest.h>
#include <mull/Mutators/CXX/RelationalMutators.h>

using namespace mull;
using namespace llvm;

TEST(Relational, LessThanToLessOrEqual) {
  auto core = init_core_ffi();
  const MullDiagnostics &diagnostics = core->diag();
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
  auto core = init_core_ffi();
  const MullDiagnostics &diagnostics = core->diag();
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
  auto core = init_core_ffi();
  const MullDiagnostics &diagnostics = core->diag();
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
  auto core = init_core_ffi();
  const MullDiagnostics &diagnostics = core->diag();
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
