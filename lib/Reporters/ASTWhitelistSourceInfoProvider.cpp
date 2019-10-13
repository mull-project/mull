//
// Created by Stanislav Pankevich on 2019-10-18.
//

#include "mull/Reporters/ASTWhitelistSourceInfoProvider.h"

#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <clang/AST/Expr.h>
#include <clang/Lex/Lexer.h>

#include <iostream>

using namespace mull;

ASTWhitelistSourceInfoProvider::ASTWhitelistSourceInfoProvider(
    ASTStorage &astStorage, ASTMutations &mutations)
    : astStorage(astStorage), mutations(mutations) {}

MutationPointSourceInfo
ASTWhitelistSourceInfoProvider::getSourceInfo(MutationPoint *mutationPoint) {
  MutationPointSourceInfo info;
  clang::SourceRange sourceRange;

  const std::string &sourceFile = mutationPoint->getSourceFileName();

  ThreadSafeASTUnit *astUnit = astStorage.findASTByPath(sourceFile);
  if (astUnit == nullptr) {
    mull::Logger::warn()
        << "error: cannot find an AST unit for mutation point\n";
    return info;
  }

  const mull::SourceLocation &sourceLocation =
      mutationPoint->getSourceLocation();

  const mull::ASTMutation &mutation = mutations.getMutation(
      sourceFile, sourceLocation.line, sourceLocation.column);

  const clang::Stmt *stmt = mutation.stmt;

  sourceRange = stmt->getSourceRange();

  clang::ASTContext &astContext = astUnit->getASTContext();

  clang::SourceManager &sourceManager = astUnit->getSourceManager();

  clang::SourceLocation sourceLocationBegin = sourceRange.getBegin();
  clang::SourceLocation sourceLocationEnd = sourceRange.getEnd();

  /// Clang AST: how to get more precise debug information in certain cases?
  /// http://clang-developers.42468.n3.nabble.com/Clang-AST-how-to-get-more-precise-debug-information-in-certain-cases-td4065195.html
  /// https://stackoverflow.com/questions/11083066/getting-the-source-behind-clangs-ast
  clang::SourceLocation sourceLocationEndActual =
      clang::Lexer::getLocForEndOfToken(sourceLocationEnd, 0, sourceManager,
                                        astContext.getLangOpts());

  info.beginColumn =
      sourceManager.getExpansionColumnNumber(sourceLocationBegin);
  info.beginLine =
      sourceManager.getExpansionLineNumber(sourceLocationBegin, nullptr);
  info.endColumn =
      sourceManager.getExpansionColumnNumber(sourceLocationEndActual);
  info.endLine =
      sourceManager.getExpansionLineNumber(sourceLocationEndActual, nullptr);

  assert(info.beginColumn < info.endColumn);
  return info;
}
