#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <cassert>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "/in-memory-file.cc";

static FilePathFilter nullPathFilter;

TEST(ASTVisitor_Comparison, equalToToNotEqual) {
  const char *const binaryOperator = R"(///
int eq(int a, int b) {
  return a == b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_EqualToNotEqual });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_EqualToNotEqual), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_EqualToNotEqual].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_EqualToNotEqual].count(locationHash), 1U);
}

TEST(ASTVisitor_Comparison, notEqualToEqual) {
  const char *const binaryOperator = R"(///
int neq(int a, int b) {
  return a != b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_NotEqualToEqual });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_NotEqualToEqual), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_NotEqualToEqual].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_NotEqualToEqual].count(locationHash), 1U);
}

TEST(ASTVisitor_Comparison, greaterThanToLessThanOrEqual) {
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

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_GreaterThanToLessOrEqual });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_GreaterThanToLessOrEqual), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterThanToLessOrEqual].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterThanToLessOrEqual].count(locationHash), 1U);
}

TEST(ASTVisitor_Comparison, lessThanToGreaterThanOrEqual) {
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

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_LessThanToGreaterOrEqual });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_LessThanToGreaterOrEqual), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessThanToGreaterOrEqual].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessThanToGreaterOrEqual].count(locationHash), 1U);
}

TEST(ASTVisitor_Comparison, greaterOrEqualToLessThan) {
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

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_GreaterOrEqualToLessThan });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_GreaterOrEqualToLessThan), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterOrEqualToLessThan].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_GreaterOrEqualToLessThan].count(locationHash), 1U);
}

TEST(ASTVisitor_Comparison, lessOrEqualToGreaterThan) {
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

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_LessOrEqualToGreaterThan });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_LessOrEqualToGreaterThan), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessOrEqualToGreaterThan].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LessOrEqualToGreaterThan].count(locationHash), 1U);
}
