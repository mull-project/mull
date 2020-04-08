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

TEST(ASTVisitor_Arithmetic, addToSub) {
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

TEST(ASTVisitor_Arithmetic, subToAdd) {
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

TEST(ASTVisitor_Arithmetic, mulToDiv) {
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

TEST(ASTVisitor_Arithmetic, divToMul) {
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

TEST(ASTVisitor_Arithmetic, remToDiv) {
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
