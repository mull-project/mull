#include "mull/JunkDetection/CXX/Visitors/InstructionRangeVisitor.h"

#include <clang/AST/Expr.h>
#include <clang/Basic/SourceManager.h>

static bool locationInRange(const clang::SourceManager &sourceManager,
                            const clang::SourceRange &range,
                            const clang::SourceLocation &location) {
  if (location.isFileID()) {
    clang::FullSourceLoc mutantLocation(location, sourceManager);
    assert(mutantLocation.isValid());
    clang::FullSourceLoc begin(range.getBegin(), sourceManager);
    assert(begin.isValid());
    bool sameSourceFile = mutantLocation.getFileID() == begin.getFileID();

    auto mutantOffset = sourceManager.getFileOffset(location);
    auto beginOffset = sourceManager.getFileOffset(range.getBegin());
    auto endOffset = sourceManager.getFileOffset(range.getEnd());

    bool inRange = (mutantOffset >= beginOffset) && (mutantOffset <= endOffset);
    return sameSourceFile && inRange;
  }
  return false;
}

bool isRangeSmallerThanRange(const clang::SourceManager &sourceManager,
                             const clang::SourceRange &first, const clang::SourceRange &second) {
  assert(first.isValid());
  assert(second.isValid());

  auto firstLength =
      sourceManager.getFileOffset(first.getEnd()) - sourceManager.getFileOffset(first.getBegin());
  auto secondLength =
      sourceManager.getFileOffset(second.getEnd()) - sourceManager.getFileOffset(second.getBegin());

  if (firstLength < secondLength) {
    return true;
  }
  if (secondLength < firstLength) {
    return false;
  }

  return true;
}

mull::InstructionRangeVisitor::InstructionRangeVisitor(const VisitorParameters &parameters)
    : sourceManager(parameters.sourceManager), sourceLocation(parameters.sourceLocation),
      bestMatchingASTNode(nullptr) {}

void mull::InstructionRangeVisitor::visitRangeWithASTExpr(const clang::Stmt *stmt) {
  assert(stmt);

  clang::SourceRange range = stmt->getSourceRange();
  if (range.isInvalid()) {
    return;
  }

  if (locationInRange(sourceManager, range, sourceLocation)) {
    if (bestMatchingASTNode == nullptr) {
      bestMatchingASTNode = stmt;
      return;
    }

    clang::SourceRange bestSourceRange = bestMatchingASTNode->getSourceRange();
    if (isRangeSmallerThanRange(sourceManager, range, bestSourceRange)) {
      bestMatchingASTNode = stmt;
    }
  }
}

const clang::Stmt *mull::InstructionRangeVisitor::getMatchingASTNode() {
  return bestMatchingASTNode;
}
