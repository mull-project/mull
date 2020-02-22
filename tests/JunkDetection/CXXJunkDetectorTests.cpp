#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"
#include "mull/ReachableFunction.h"
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/Mutators/CXX/ArithmeticMutators.h>
#include <mull/Mutators/CXX/BitwiseMutators.h>
#include <mull/Mutators/CXX/LogicalAndToOr.h>
#include <mull/Mutators/CXX/LogicalOrToAnd.h>
#include <mull/Mutators/CXX/NumberMutators.h>
#include <mull/Mutators/CXX/RelationalMutators.h>

#include <gtest/gtest.h>
#include <llvm/IR/LLVMContext.h>

using namespace mull;
using namespace llvm;

using ::testing::TestWithParam;
using ::testing::Values;

struct CXXJunkDetectorTestParameter {
  const char *bitcodePath;
  Mutator *mutator;
  uint8_t nonJunkMutants;
  CXXJunkDetectorTestParameter(const char *bitcodePath, Mutator *mutator, uint8_t nonJunkMutants)
      : bitcodePath(bitcodePath), mutator(mutator), nonJunkMutants(nonJunkMutants) {}

  friend std::ostream &operator<<(std::ostream &os, const CXXJunkDetectorTestParameter &bar) {
    os << "path(" << bar.bitcodePath << ") mutator(" << bar.mutator->getUniqueIdentifier()
       << ") non-junk-mutants(" << std::to_string(bar.nonJunkMutants) << ")";
    return os;
  }
};

class CXXJunkDetectorTest : public TestWithParam<CXXJunkDetectorTestParameter> {};

TEST_P(CXXJunkDetectorTest, detectJunk) {
  Diagnostics diagnostics;
  auto &parameter = GetParam();
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(parameter.bitcodePath, context, diagnostics);

  std::vector<MutationPoint *> points;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = parameter.mutator->getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASTStorage astStorage(diagnostics, "", "");
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

  CXXJunkDetectorTestParameter(fixtures::mutators_math_add_module_bc_path(), new cxx::AddToSub, 6),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_add_module_bc_path(),
                               new cxx::AddAssignToSubAssign, 6),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_add_module_bc_path(),
                               new cxx::PreIncToPreDec, 1),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_add_module_bc_path(),
                               new cxx::PostIncToPostDec, 3),

  CXXJunkDetectorTestParameter(fixtures::mutators_math_mul_junk_bc_path(), new cxx::MulToDiv, 8),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_mul_junk_bc_path(),
                               new cxx::MulAssignToDivAssign, 2),

  CXXJunkDetectorTestParameter(fixtures::mutators_math_div_junk_bc_path(), new cxx::DivToMul, 8),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_div_junk_bc_path(),
                               new cxx::DivAssignToMulAssign, 2),

  CXXJunkDetectorTestParameter(fixtures::mutators_rem_to_div_junk_bc_path(), new cxx::RemToDiv, 5),
  CXXJunkDetectorTestParameter(fixtures::mutators_rem_to_div_junk_bc_path(),
                               new cxx::RemAssignToDivAssign, 3),

  CXXJunkDetectorTestParameter(fixtures::mutators_math_sub_junk_bc_path(), new cxx::SubToAdd, 5),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_sub_junk_bc_path(),
                               new cxx::SubAssignToAddAssign, 3),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_sub_junk_bc_path(), new cxx::PreDecToPreInc,
                               1),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_sub_junk_bc_path(),
                               new cxx::PostDecToPostInc, 3),

  CXXJunkDetectorTestParameter(fixtures::mutators_math_bitwise_not_bc_path(),
                               new cxx::BitwiseNotToNoop, 2),
  CXXJunkDetectorTestParameter(fixtures::mutators_math_unary_minus_bc_path(),
                               new cxx::UnaryMinusToNoop, 4),

  /// Shifts
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_shifts_bc_path(), new cxx::LShiftToRShift,
                               4),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_shifts_bc_path(),
                               new cxx::LShiftAssignToRShiftAssign, 5),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_shifts_bc_path(), new cxx::RShiftToLShift,
                               3),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_shifts_bc_path(),
                               new cxx::RShiftAssignToLShiftAssign, 3),
  /// Bit operations
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_bitops_bc_path(), new cxx::BitwiseOrToAnd,
                               2),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_bitops_bc_path(),
                               new cxx::OrAssignToAndAssign, 1),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_bitops_bc_path(), new cxx::BitwiseAndToOr,
                               2),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_bitops_bc_path(),
                               new cxx::AndAssignToOrAssign, 2),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_bitops_bc_path(), new cxx::XorToOr, 2),
  CXXJunkDetectorTestParameter(fixtures::mutators_bitwise_bitops_bc_path(),
                               new cxx::XorAssignToOrAssign, 3),

  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new NegateConditionMutator, 6),

  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new cxx::EqualToNotEqual, 3),
  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new cxx::NotEqualToEqual, 6),

  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new cxx::GreaterThanToLessOrEqual, 6),
  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new cxx::GreaterOrEqualToLessThan, 3),
  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new cxx::LessThanToGreaterOrEqual, 3),
  CXXJunkDetectorTestParameter(fixtures::mutators_negate_condition_junk_bc_path(),
                               new cxx::LessOrEqualToGreaterThan, 3),

  CXXJunkDetectorTestParameter(fixtures::mutators_replace_assignment_junk_bc_path(),
                               new cxx::NumberAssignConst, 5),
  CXXJunkDetectorTestParameter(fixtures::mutators_replace_assignment_junk_bc_path(),
                               new cxx::NumberInitConst, 6),

  CXXJunkDetectorTestParameter(fixtures::mutators_and_or_and_to_or_replacement_cpp_junk_bc_path(),
                               new cxx::LogicalAndToOr, 2),

  CXXJunkDetectorTestParameter(fixtures::mutators_and_or_or_to_and_replacement_cpp_junk_bc_path(),
                               new cxx::LogicalOrToAnd, 2),

  CXXJunkDetectorTestParameter(fixtures::mutators_scalar_value_junk_bc_path(),
                               new ScalarValueMutator, 5),

  CXXJunkDetectorTestParameter(fixtures::mutators_remove_void_function_junk_bc_path(),
                               new RemoveVoidFunctionMutator, 6),
  CXXJunkDetectorTestParameter(fixtures::mutators_replace_call_junk_bc_path(),
                               new ReplaceCallMutator, 11),
};

INSTANTIATE_TEST_CASE_P(CXXJunkDetection, CXXJunkDetectorTest, testing::ValuesIn(parameters));

TEST(CXXJunkDetector, compdb_absolute_paths) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::junk_detection_compdb_main_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(new cxx::LessOrEqualToLessThan);
  mutators.emplace_back(new cxx::LessThanToLessOrEqual);
  mutators.emplace_back(new cxx::GreaterOrEqualToGreaterThan);
  mutators.emplace_back(new cxx::GreaterThanToGreaterOrEqual);
  for (auto &mutator : mutators) {
    for (auto &function : bitcode->getModule()->functions()) {
      FunctionUnderTest functionUnderTest(&function, nullptr, 0);
      functionUnderTest.selectInstructions({});
      auto mutants = mutator->getMutations(bitcode.get(), functionUnderTest);
      std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
    }
  }

  ASSERT_EQ(points.size(), 8U);

  std::string cxxCompilationDatabasePath =
      fixtures::junk_detection_compdb_absolute_compile_commands_json_path();

  ASTStorage astStorage(diagnostics, cxxCompilationDatabasePath, "");
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
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::junk_detection_compdb_main_bc_path(), context, diagnostics);

  std::vector<MutationPoint *> points;
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(new cxx::LessOrEqualToLessThan);
  mutators.emplace_back(new cxx::LessThanToLessOrEqual);
  mutators.emplace_back(new cxx::GreaterOrEqualToGreaterThan);
  mutators.emplace_back(new cxx::GreaterThanToGreaterOrEqual);

  for (auto &mutator : mutators) {
    for (auto &function : bitcode->getModule()->functions()) {
      FunctionUnderTest functionUnderTest(&function, nullptr, 0);
      auto mutants = mutator->getMutations(bitcode.get(), functionUnderTest);
      std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
    }
  }

  ASSERT_EQ(points.size(), 8U);

  std::string cxxCompilationDatabasePath =
      fixtures::junk_detection_compdb_relative_compile_commands_json_path();

  ASTStorage astStorage(diagnostics, cxxCompilationDatabasePath, "");
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
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::junk_detection_compdb_main_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(new cxx::LessOrEqualToLessThan);
  mutators.emplace_back(new cxx::LessThanToLessOrEqual);
  mutators.emplace_back(new cxx::GreaterOrEqualToGreaterThan);
  mutators.emplace_back(new cxx::GreaterThanToGreaterOrEqual);

  for (auto &mutator : mutators) {
    for (auto &function : bitcode->getModule()->functions()) {
      FunctionUnderTest functionUnderTest(&function, nullptr, 0);
      functionUnderTest.selectInstructions({});
      auto mutants = mutator->getMutations(bitcode.get(), functionUnderTest);
      std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
    }
  }

  ASSERT_EQ(points.size(), 8U);

  std::string cxxCompilationFlags =
      std::string("-I ") + fixtures::junk_detection_compdb_include__path();

  ASTStorage astStorage(diagnostics, "", cxxCompilationFlags);
  CXXJunkDetector detector(astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}
