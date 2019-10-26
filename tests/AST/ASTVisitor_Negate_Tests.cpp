#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "input.cc";
static FilePathFilter nullPathFilter;

TEST(ASTVisitorTest, negateTest_negativeToPositive) {
  const char *const binaryOperator = R"(///
int foo(int a) {
  return !a;
};
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::NEGATE);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation
    &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 11);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  astMutations.dump();

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 11));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 11U);
}

TEST(ASTVisitorTest, negateTest_positiveToNegative) {
  const char *const binaryOperator = R"(///
int foo(int a) {
  return a;
};
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::NEGATE);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 10);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  astMutations.dump();

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 10));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 10U);
}

