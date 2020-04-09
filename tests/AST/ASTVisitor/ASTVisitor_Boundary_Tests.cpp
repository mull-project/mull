#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "/in-memory-file.cc";

static FilePathFilter nullPathFilter;

TEST(ASTVisitor_Arithmetic, greaterThanToGreaterThanOrEqual) {
  const char *const binaryOperator = R"(///
int greater_than(int a, int b) {
  return a > b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_GreaterThanToGreaterOrEqual });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_GreaterThanToGreaterOrEqual), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterThanToGreaterOrEqual].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterThanToGreaterOrEqual].count(locationHash), 1U);
}

TEST(ASTVisitor_Arithmetic, lessThanToLessThanOrEqual) {
  const char *const binaryOperator = R"(///
int less_than(int a, int b) {
  return a < b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_LessThanToLessOrEqual });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_LessThanToLessOrEqual), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessThanToLessOrEqual].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessThanToLessOrEqual].count(locationHash), 1U);
}

TEST(ASTVisitor_Arithmetic, greaterThanOrEqualToGreaterThan) {
  const char *const binaryOperator = R"(///
int greater_than_or_equal(int a, int b) {
  return a >= b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_GreaterOrEqualToGreaterThan });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_GreaterOrEqualToGreaterThan), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterOrEqualToGreaterThan].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterOrEqualToGreaterThan].count(locationHash), 1U);
}

TEST(ASTVisitor_Arithmetic, lessThanOrEqualToLessThan) {
  const char *const binaryOperator = R"(///
int less_than_or_equal(int a, int b) {
  return a <= b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_LessOrEqualToLessThan });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_LessOrEqualToLessThan), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessOrEqualToLessThan].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessOrEqualToLessThan].count(locationHash), 1U);
}
