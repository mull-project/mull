#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "input.cc";
static FilePathFilter nullPathFilter;

TEST(ASTVisitorTest, AND_to_OR) {
  const char *const binaryOperator = R"(///
int foo(int a, int b) {
  return a && b;
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::AND_OR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 12);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 12));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 10U);
  ASSERT_EQ(mutation.line, 3U);
  ASSERT_EQ(mutation.column, 12U);
}
