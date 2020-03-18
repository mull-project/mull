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

TEST(ASTVisitorTest, binaryAddOperator) {
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
