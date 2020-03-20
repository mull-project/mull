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

TEST(ASTVisitorTest, binaryAddToSubOperator) {
  const char *const binaryOperator = R"(///
int add(int a, int b) {
  return a + b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_AddToSub });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_AddToSub), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddToSub].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddToSub].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binarySubToAddOperator) {
  const char *const binaryOperator = R"(///
int sub(int a, int b) {
  return a - b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_SubToAdd });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_SubToAdd), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_SubToAdd].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_SubToAdd].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binaryMulToDivOperator) {
  const char *const binaryOperator = R"(///
int mul(int a, int b) {
  return a * b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_MulToDiv });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_MulToDiv), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_MulToDiv].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_MulToDiv].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binaryDivToMullOperator) {
  const char *const binaryOperator = R"(///
int mul(int a, int b) {
  return a / b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_DivToMul });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_DivToMul), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_DivToMul].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_DivToMul].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binaryRemToDivOperator) {
  const char *const binaryOperator = R"(///
int rem(int a, int b) {
  return a % b;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_RemToDiv });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_RemToDiv), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_RemToDiv].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_RemToDiv].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binaryAddAssignToSubAssignOperator) {
  const char *const binaryOperator = R"(///
int add_assign(int a, int b) {
  a += b;
  return a;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_AddAssignToSubAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_AddAssignToSubAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddAssignToSubAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddAssignToSubAssign].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binarySubAssignToAddAssignOperator) {
  const char *const binaryOperator = R"(///
int sub_assign(int a, int b) {
  a -= b;
  return a;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_SubAssignToAddAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_SubAssignToAddAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_SubAssignToAddAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_SubAssignToAddAssign].count(locationHash), 1U);
}

TEST(ASTVisitorTest, binaryMulAssignToDivAssignOperator) {
  const char *const binaryOperator = R"(///
int mul_assign(int a, int b) {
  a *= b;
  return a;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_MulAssignToDivAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_MulAssignToDivAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_MulAssignToDivAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_MulAssignToDivAssign].count(locationHash), 1U);
}
