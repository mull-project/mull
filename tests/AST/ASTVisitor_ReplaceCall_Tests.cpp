#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "input.cc";
static FilePathFilter nullPathFilter;

TEST(ASTVisitorTest, replaceCall) {
  const char *const binaryOperator = R"(///
int callee(int a) {
  return a;
}
int caller(int a) {
  return callee(a);
};
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::REPLACE_CALL);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation
    &mutation = astMutations.getMutation(fakeSourceFilePath, 6, 10);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  astMutations.dump();

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 6, 10));
  ASSERT_EQ(clangLine, 6U);
  ASSERT_EQ(clangColumn, 10U);
}
