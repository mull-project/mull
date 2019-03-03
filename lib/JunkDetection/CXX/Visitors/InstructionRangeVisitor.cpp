#include "JunkDetection/CXX/Visitors/InstructionRangeVisitor.h"

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

static clang::SourceRange
smallestSourceRange(const clang::SourceManager &sourceManager,
                    const clang::SourceRange &first,
                    const clang::SourceRange &second) {
  if (first.isInvalid()) {
    return second;
  }
  if (second.isInvalid()) {
    return first;
  }

  auto firstLength = sourceManager.getFileOffset(first.getEnd()) -
      sourceManager.getFileOffset(first.getBegin());
  auto secondLength = sourceManager.getFileOffset(second.getEnd()) -
      sourceManager.getFileOffset(second.getBegin());

  if (firstLength < secondLength) {
    return first;
  }
  if (secondLength < firstLength) {
    return second;
  }

  return first;
}

mull::InstructionRangeVisitor::InstructionRangeVisitor(
    const clang::SourceManager &sourceManager,
    const clang::SourceLocation &sourceLocation)
    : sourceManager(sourceManager), sourceLocation(sourceLocation),
      sourceRange() {}

void mull::InstructionRangeVisitor::visitRangeWithLocation(
    const clang::SourceRange &range) {
  if (locationInRange(sourceManager, range, sourceLocation)) {
    sourceRange = smallestSourceRange(sourceManager, sourceRange, range);
  }
}

bool mull::InstructionRangeVisitor::foundRange() {
  return sourceRange.isValid();
}
