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

TEST(ASTVisitor_ReplaceCall, test) {
  const char *const binaryOperator = R"(///
int callee(int a) {
  return a;
}
int caller(int a) {
  return callee(a);
};
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::ReplaceCallMutator });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(6, 10);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::ReplaceCallMutator), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::ReplaceCallMutator].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::ReplaceCallMutator].count(locationHash), 1U);
}
