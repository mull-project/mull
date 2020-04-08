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

TEST(ASTVisitor_BitwiseAssignment, bitwiseAndAssignToOrAssign) {
  const char *const binaryOperator = R"(///
int bitwise_and_assign(int a, int b) {
  int c = a;
  c &= b;
  return c;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_Bitwise_AndAssignToOrAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(4, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_Bitwise_AndAssignToOrAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_AndAssignToOrAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_AndAssignToOrAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_BitwiseAssignment, bitwiseOrAssignToAndAssign) {
  const char *const binaryOperator = R"(///
int bitwise_or_assign(int a, int b) {
  int c = a;
  c |= b;
  return c;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_Bitwise_OrAssignToAndAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(4, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_Bitwise_OrAssignToAndAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_OrAssignToAndAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_OrAssignToAndAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_BitwiseAssignment, bitwiseXorAssignToOrAssign) {
  const char *const binaryOperator = R"(///
int bitwise_xor_assign(int a, int b) {
  int c = a;
  c ^= b;
  return c;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_Bitwise_XorAssignToOrAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(4, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_Bitwise_XorAssignToOrAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_XorAssignToOrAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_Bitwise_XorAssignToOrAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_BitwiseAssignment, bitwiseLeftShiftAssignToRightAssign) {
  const char *const binaryOperator = R"(///
int bitwise_left_shift_assign(int a, int b) {
  int c = a;
  c <<= b;
  return c;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_LShiftAssignToRShiftAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(4, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_LShiftAssignToRShiftAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LShiftAssignToRShiftAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_LShiftAssignToRShiftAssign].count(locationHash), 1U);
}

TEST(ASTVisitor_BitwiseAssignment, bitwiseRightShiftAssignToLeftShiftAssign) {
  const char *const binaryOperator = R"(///
int bitwise_right_shift_assign(int a, int b) {
  int c = a;
  c >>= b;
  return c;
}
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_RShiftAssignToLShiftAssign });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
    diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(4, 5);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_RShiftAssignToLShiftAssign), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_RShiftAssignToLShiftAssign].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_RShiftAssignToLShiftAssign].count(locationHash), 1U);
}
