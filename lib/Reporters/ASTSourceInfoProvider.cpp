#include "mull/Reporters/ASTSourceInfoProvider.h"

#include "mull/AST/ASTMutationStorage.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/Mutant.h"
#include "mull/SourceLocation.h"

#include <clang/AST/Expr.h>
#include <clang/Lex/Lexer.h>

using namespace mull;

ASTSourceInfoProvider::ASTSourceInfoProvider(ASTStorage &astStorage) : astStorage(astStorage) {}

MutationPointSourceInfo ASTSourceInfoProvider::getSourceInfo(Diagnostics &diagnostics,
                                                             Mutant *mutant) {
  MutationPointSourceInfo info = MutationPointSourceInfo();
  clang::SourceRange sourceRange;

  const SourceLocation &sourceLocation = mutant->getSourceLocation();
  const std::string &sourceFile = sourceLocation.unitFilePath;

  const ASTMutation &astMutation = astStorage.getMutation(
      sourceFile, mutant->getMutatorKind(), sourceLocation.line, sourceLocation.column);

  const clang::Stmt *const mutantASTNode = astMutation.stmt;

  ThreadSafeASTUnit *astUnit = astStorage.findAST(mutant->getSourceLocation());

  if (mutantASTNode == nullptr) {
    diagnostics.warning("Cannot find an AST node for mutation point");
    return info;
  }
  if (astUnit == nullptr) {
    diagnostics.warning("Cannot find an AST unit for mutation point");
    return info;
  }

  sourceRange = mutantASTNode->getSourceRange();

  clang::ASTContext &astContext = astUnit->getASTContext();

  clang::SourceManager &sourceManager = astUnit->getSourceManager();

  clang::SourceLocation sourceLocationBegin = sourceRange.getBegin();
  clang::SourceLocation sourceLocationEnd = sourceRange.getEnd();

  /// Clang AST: how to get more precise debug information in certain cases?
  /// http://clang-developers.42468.n3.nabble.com/Clang-AST-how-to-get-more-precise-debug-information-in-certain-cases-td4065195.html
  /// https://stackoverflow.com/questions/11083066/getting-the-source-behind-clangs-ast
  clang::SourceLocation sourceLocationEndActual = clang::Lexer::getLocForEndOfToken(
      sourceLocationEnd, 0, sourceManager, astContext.getLangOpts());

  info.beginColumn = sourceManager.getExpansionColumnNumber(sourceLocationBegin);
  info.beginLine = sourceManager.getExpansionLineNumber(sourceLocationBegin, nullptr);
  info.endColumn = sourceManager.getExpansionColumnNumber(sourceLocationEndActual);
  info.endLine = sourceManager.getExpansionLineNumber(sourceLocationEndActual, nullptr);

  return info;
}
