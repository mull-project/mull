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

TEST(ASTVisitor_RemoveVoid, test) {
  const char *const binaryOperator = R"(///
void voidFunction() {}
void foo() {
  voidFunction();
};
)";

  Diagnostics diagnostics;
  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));
  assert(astUnit);

  SingleASTUnitMutations singleUnitMutations;

  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_RemoveVoidCall });

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(4, 3);

  ASSERT_EQ(singleUnitMutations.size(), 1U);

  SingleFileMutations &singleFileMutations = singleUnitMutations[fakeSourceFilePath];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_RemoveVoidCall), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_RemoveVoidCall].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_RemoveVoidCall].count(locationHash), 1U);
}
