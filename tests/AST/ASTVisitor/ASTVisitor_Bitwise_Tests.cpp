#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "input.cc";

static FilePathFilter nullPathFilter;

TEST(ASTVisitor_Bitwise, bitwiseAndToOr) {
  const char *const binaryOperator = R"(///
int bitwise_and(int a, int b) {
  return a & b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_Bitwise_AndToOr });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_Bitwise_AndToOr), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_AndToOr].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_AndToOr].count(locationHash), 1U);
}

TEST(ASTVisitor_Bitwise, bitwiseOrToAnd) {
  const char *const binaryOperator = R"(///
int bitwise_or(int a, int b) {
  return a | b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_Bitwise_OrToAnd });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_Bitwise_OrToAnd), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_OrToAnd].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_OrToAnd].count(locationHash), 1U);
}

TEST(ASTVisitor_Bitwise, bitwiseXorToOr) {
  const char *const binaryOperator = R"(///
int bitwise_xor(int a, int b) {
  return a ^ b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_Bitwise_XorToOr });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_Bitwise_XorToOr), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_XorToOr].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_XorToOr].count(locationHash), 1U);
}

TEST(ASTVisitor_Bitwise, bitwiseLeftShiftToRight) {
  const char *const binaryOperator = R"(///
int bitwise_left_shift(int a, int b) {
  return a << b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_LShiftToRShift });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_LShiftToRShift), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LShiftToRShift].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LShiftToRShift].count(locationHash), 1U);
}
