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

TEST(ASTVisitor_ArithmeticAssignment, addAssignToSubAssign) {
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

  SingleFileMutations &singleFileMutations = singleUnitMutations[fakeSourceFilePath];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_AddAssignToSubAssign), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_AddAssignToSubAssign].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_AddAssignToSubAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_ArithmeticAssignment, subAssignToAddAssign) {
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

  SingleFileMutations &singleFileMutations = singleUnitMutations[fakeSourceFilePath];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_SubAssignToAddAssign), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_SubAssignToAddAssign].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_SubAssignToAddAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_ArithmeticAssignment, mulAssignToDivAssign) {
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

  SingleFileMutations &singleFileMutations = singleUnitMutations[fakeSourceFilePath];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_MulAssignToDivAssign), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_MulAssignToDivAssign].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_MulAssignToDivAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_ArithmeticAssignment, divAssignToMulAssign) {
  const char *const binaryOperator = R"(///
int div_assign(int a, int b) {
  a /= b;
  return a;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_DivAssignToMulAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);

  SingleFileMutations &singleFileMutations = singleUnitMutations[fakeSourceFilePath];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_DivAssignToMulAssign), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_DivAssignToMulAssign].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_DivAssignToMulAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_ArithmeticAssignment, remAssignToDivAssign) {
  const char *const binaryOperator = R"(///
int div_assign(int a, int b) {
  a %= b;
  return a;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_RemAssignToDivAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);

  SingleFileMutations &singleFileMutations = singleUnitMutations[fakeSourceFilePath];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_RemAssignToDivAssign), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_RemAssignToDivAssign].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_RemAssignToDivAssign].count(locationHash), 1U);
}
