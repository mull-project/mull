#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/RawConfig.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/AndOrReplacementMutator.h"
#include "mull/Mutators/MathAddMutator.h"
#include "mull/Mutators/MathDivMutator.h"
#include "mull/Mutators/MathMulMutator.h"
#include "mull/Mutators/MathSubMutator.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/Mutators/ReplaceAssignmentMutator.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"
#include <mull/Mutators/CXX/RelationalMutators.h>

#include <gtest/gtest.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

using namespace mull;
using namespace llvm;

using ::testing::TestWithParam;
using ::testing::Values;

struct CXXJunkDetectorTestParameter {
  const char *bitcodePath;
  Mutator *mutator;
  uint8_t nonJunkMutants;
  CXXJunkDetectorTestParameter(const char *bitcodePath, Mutator *mutator,
                               uint8_t nonJunkMutants)
      : bitcodePath(bitcodePath), mutator(mutator),
        nonJunkMutants(nonJunkMutants) {}

  friend std::ostream &operator<<(std::ostream &os,
                                  const CXXJunkDetectorTestParameter &bar) {
    os << "path(" << bar.bitcodePath << ") mutator("
       << bar.mutator->getUniqueIdentifier() << ") non-junk-mutants("
       << std::to_string(bar.nonJunkMutants) << ")";
    return os;
  }
};

class CXXJunkDetectorTest : public TestWithParam<CXXJunkDetectorTestParameter> {
};

TEST_P(CXXJunkDetectorTest, detectJunk) {
  auto &parameter = GetParam();
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(parameter.bitcodePath, context);

  std::vector<MutationPoint *> points;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = parameter.mutator->getMutations(bitcode.get(), &function);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  JunkDetectionConfig junkConfig;

  ASTStorage astStorage(junkConfig.cxxCompilationDatabasePath,
                        junkConfig.cxxCompilationFlags);
  CXXJunkDetector detector(astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), parameter.nonJunkMutants);
}

static const CXXJunkDetectorTestParameter parameters[] = {
    CXXJunkDetectorTestParameter(fixtures::mutators_boundary_module_bc_path(),
                                 new cxx::LessThanToLessOrEqual, 3),
    CXXJunkDetectorTestParameter(fixtures::mutators_boundary_module_bc_path(),
                                 new cxx::LessOrEqualToLessThan, 1),
    CXXJunkDetectorTestParameter(fixtures::mutators_boundary_module_bc_path(),
                                 new cxx::GreaterThanToGreaterOrEqual, 1),
    CXXJunkDetectorTestParameter(fixtures::mutators_boundary_module_bc_path(),
                                 new cxx::GreaterOrEqualToGreaterThan, 1),
    CXXJunkDetectorTestParameter(fixtures::mutators_math_add_module_bc_path(),
                                 new MathAddMutator, 16),
    CXXJunkDetectorTestParameter(fixtures::mutators_math_mul_junk_bc_path(),
                                 new MathMulMutator, 10),
    CXXJunkDetectorTestParameter(fixtures::mutators_math_div_junk_bc_path(),
                                 new MathDivMutator, 10),

    /// FIXME: Here MathSub should produce 14 mutants, but only 9 found because
    /// some of the '--/-= 1' instructions converted into 'add -1'
    /// There are three possible solutions:
    ///   1. Consider 'add' and 'sub' instruction as MathSub and filter out junk
    ///      later. This way we must rely on JunkDetector, otherwise we get lots
    ///      of duplicates
    ///   2. Add an extra check to the MathSub, i.e. an instruction is MathSub
    ///      if it is 'add' and one of the operands is '-1'. Then, to apply a
    ///      mutation we would need to replace -1 with 1.
    ///   3. Merge MathAdd and MathSub into one mutator. Less granulation, but
    ///      higher quality of mutants
    ///
    /// At the moment of writing the second option more.
    ///
    CXXJunkDetectorTestParameter(fixtures::mutators_math_sub_junk_bc_path(),
                                 new MathSubMutator, 8),
    CXXJunkDetectorTestParameter(
        fixtures::mutators_remove_void_function_junk_bc_path(),
        new RemoveVoidFunctionMutator, 6),
    CXXJunkDetectorTestParameter(fixtures::mutators_replace_call_junk_bc_path(),
                                 new ReplaceCallMutator, 11),

    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new NegateConditionMutator, 6),

    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new cxx::EqualToNotEqual, 3),
    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new cxx::NotEqualToEqual, 6),

    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new cxx::GreaterThanToLessOrEqual, 6),
    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new cxx::GreaterOrEqualToLessThan, 3),
    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new cxx::LessThanToGreaterOrEqual, 3),
    CXXJunkDetectorTestParameter(
        fixtures::mutators_negate_condition_junk_bc_path(),
        new cxx::LessOrEqualToGreaterThan, 3),

    CXXJunkDetectorTestParameter(
        fixtures::mutators_and_or_replacement_cpp_junk_bc_path(),
        new AndOrReplacementMutator, 4),
    CXXJunkDetectorTestParameter(fixtures::mutators_scalar_value_junk_bc_path(),
                                 new ScalarValueMutator, 5),
    CXXJunkDetectorTestParameter(
        fixtures::mutators_replace_assignment_junk_bc_path(),
        new ReplaceAssignmentMutator, 5),
};

INSTANTIATE_TEST_CASE_P(CXXJunkDetection, CXXJunkDetectorTest,
                        testing::ValuesIn(parameters));

TEST(CXXJunkDetector, compdb_absolute_paths) {
  LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::junk_detection_compdb_main_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(new cxx::LessOrEqualToLessThan);
  mutators.emplace_back(new cxx::LessThanToLessOrEqual);
  mutators.emplace_back(new cxx::GreaterOrEqualToGreaterThan);
  mutators.emplace_back(new cxx::GreaterThanToGreaterOrEqual);
  for (auto &mutator : mutators) {
    for (auto &function : bitcode->getModule()->functions()) {
      auto mutants = mutator->getMutations(bitcode.get(), &function);
      std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
    }
  }

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompilationDatabasePath =
      fixtures::junk_detection_compdb_absolute_compile_commands_json_path();

  ASTStorage astStorage(junkConfig.cxxCompilationDatabasePath,
                        junkConfig.cxxCompilationFlags);
  CXXJunkDetector detector(astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}

TEST(CXXJunkDetector, DISABLED_compdb_relative_paths) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::junk_detection_compdb_main_bc_path(), context);

  std::vector<MutationPoint *> points;
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(new cxx::LessOrEqualToLessThan);
  mutators.emplace_back(new cxx::LessThanToLessOrEqual);
  mutators.emplace_back(new cxx::GreaterOrEqualToGreaterThan);
  mutators.emplace_back(new cxx::GreaterThanToGreaterOrEqual);

  for (auto &mutator : mutators) {
    for (auto &function : bitcode->getModule()->functions()) {
      auto mutants = mutator->getMutations(bitcode.get(), &function);
      std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
    }
  }

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompilationDatabasePath =
      fixtures::junk_detection_compdb_relative_compile_commands_json_path();

  ASTStorage astStorage(junkConfig.cxxCompilationDatabasePath,
                        junkConfig.cxxCompilationFlags);
  CXXJunkDetector detector(astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}

TEST(CXXJunkDetector, no_compdb) {
  LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::junk_detection_compdb_main_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(new cxx::LessOrEqualToLessThan);
  mutators.emplace_back(new cxx::LessThanToLessOrEqual);
  mutators.emplace_back(new cxx::GreaterOrEqualToGreaterThan);
  mutators.emplace_back(new cxx::GreaterThanToGreaterOrEqual);

  for (auto &mutator : mutators) {
    for (auto &function : bitcode->getModule()->functions()) {
      auto mutants = mutator->getMutations(bitcode.get(), &function);
      std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
    }
  }

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompilationFlags =
      std::string("-I ") + fixtures::junk_detection_compdb_include__path();

  ASTStorage astStorage(junkConfig.cxxCompilationDatabasePath,
                        junkConfig.cxxCompilationFlags);
  CXXJunkDetector detector(astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}
