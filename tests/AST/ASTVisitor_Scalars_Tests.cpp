#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <clang/Tooling/Tooling.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static const char *const fakeSourceFilePath = "input.cc";
static FilePathFilter nullPathFilter;

TEST(ASTVisitorTest, returnValueIntegerScalar) {
  const char *const binaryOperator = R"(///
int add() {
  return 5;
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 3));
}

TEST(ASTVisitorTest, integerScalarWithBinaryOperator) {
  const char *const binaryOperator = R"(///
int add(int a) {
  return a + 5;
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 12);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 12));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 14U);
  ASSERT_EQ(mutation.line, 3U);
  ASSERT_EQ(mutation.column, 12U);
}

TEST(ASTVisitorTest, integerScalarAsAnArgumentToAFunctionCall) {
  const char *const binaryOperator = R"(///
void callee(int a) {
  return;
}
void caller() {
  callee(5);
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
      clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 6, 3);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 6, 3));
  ASSERT_EQ(clangLine, 6U);
  ASSERT_EQ(clangColumn, 10U);
  ASSERT_EQ(mutation.line, 6);
  ASSERT_EQ(mutation.column, 3);
}


TEST(ASTVisitorTest, integerScalarAsAnArgumentToAFunctionCall_implicitCast) {
  const char *const binaryOperator = R"(///
int at(unsigned int a) {
  return a;
}

static int func() {
  int result = at(0);
  return result;
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  astMutations.dump();

  ASSERT_EQ(astMutations.count(), 1U);


  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 7, 16));

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 7, 16);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(clangLine, 7U);
  ASSERT_EQ(clangColumn, 19U);
  ASSERT_EQ(mutation.line, 7);
  ASSERT_EQ(mutation.column, 16);
}

TEST(ASTVisitorTest, integerScalarAsAnArgumentToACXXMemberFunctionCall) {
  const char *const binaryOperator = R"(///
class String {
public:
  int at(int a) {
    return a;
  }
};
static bool func(String &line) {
  if (line.at(0)) {
    return func(line);
  }
  return func(line);
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  astMutations.dump();

  ASSERT_EQ(astMutations.count(), 1U);


  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 9, 12));

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 9, 12);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(clangLine, 9U);
  ASSERT_EQ(clangColumn, 15U);
}

TEST(ASTVisitorTest, characterLiteral) {
  const char *const binaryOperator = R"(///
int add(int a) {
  return a == '<';
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 12);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 12));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 15U);
}

TEST(ASTVisitorTest, CStyleCastExpression) {
  const char *const binaryOperator = R"(///
int add(int a) {
  return a == (int)'<';
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 12);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 12));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 20U);
}

TEST(ASTVisitorTest,constructorDeclaration) {
  const char *const binaryOperator = R"(///
class Foo {
int a;
public:
  Foo(): a(1) {}
};
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 5, 10);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 5, 10));
  ASSERT_EQ(clangLine, 5U);
  ASSERT_EQ(clangColumn, 12U);
}

TEST(ASTVisitorTest, variableDeclaration) {
  const char *const binaryOperator = R"(///
void foo() {
  int a = 1;
}
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 3, 7);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 3, 7));
  ASSERT_EQ(clangLine, 3U);
  ASSERT_EQ(clangColumn, 11U);
}

TEST(ASTVisitorTest, cxxTemporaryObjectExpr) {
  const char *const binaryOperator = R"(///
struct X {
  X(int, char *);
};

X create_X() {
  const char str[] = "not relevant to the test";
  return X(1234, (char *)str); // creates a CXXTemporaryObjectExpr
};
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 8, 10);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 8, 10));
  ASSERT_EQ(clangLine, 8U);
  ASSERT_EQ(clangColumn, 12U);
}

TEST(ASTVisitorTest, enumValue) {
  const char *const binaryOperator = R"(///
enum Enum { SUCCESS = 1234 };

Enum foo() {
  return SUCCESS;
};
)";

  ASTStorage storage("", "");

  std::unique_ptr<clang::ASTUnit> astUnit(
    clang::tooling::buildASTFromCode(binaryOperator, fakeSourceFilePath));

  ASTMutations astMutations;

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));
  ASTVisitor astVisitor(threadSafeAstUnit, astMutations,
                        nullPathFilter,
                        mull::TraverseMask::SCALAR);

  astVisitor.traverse();

  clang::SourceManager &sourceManager = threadSafeAstUnit.getSourceManager();

  const ASTMutation &mutation = astMutations.getMutation(fakeSourceFilePath, 5, 3);

  clang::SourceLocation begin = mutation.stmt->getSourceRange().getBegin();

  int clangLine = sourceManager.getExpansionLineNumber(begin);
  int clangColumn = sourceManager.getExpansionColumnNumber(begin);

  ASSERT_EQ(astMutations.count(), 1U);
  ASSERT_TRUE(astMutations.locationExists(fakeSourceFilePath, 5, 3));
  ASSERT_EQ(clangLine, 5U);
  ASSERT_EQ(clangColumn, 10U);
}
