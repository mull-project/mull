#include "JunkDetection/CXX/CXXJunkDetector.h"

#include "Config/RawConfig.h"
#include "MutationPoint.h"
#include "Mutators/Mutator.h"

#include "JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"
#include "JunkDetection/CXX/Visitors/MathAddVisitor.h"
#include "JunkDetection/CXX/Visitors/MathSubVisitor.h"
#include "JunkDetection/CXX/Visitors/NegateConditionVisitor.h"
#include "JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.h"

using namespace mull;
using namespace llvm;

CXXJunkDetector::CXXJunkDetector(JunkDetectionConfig &config)
    : astStorage(config.cxxCompDBDirectory, config.cxxCompilationFlags) {}

bool CXXJunkDetector::isJunk(MutationPoint *point) {
  auto sourceLocation = point->getSourceLocation();
  if (sourceLocation.isNull()) {
    return true;
  }

  switch (point->getMutator()->mutatorKind()) {
  case MutatorKind::ConditionalsBoundaryMutator:
    return isJunkBoundaryConditional(point, sourceLocation);
  case MutatorKind::MathAddMutator:
    return isJunkMathAdd(point, sourceLocation);
  case MutatorKind::MathSubMutator:
    return isJunkMathSub(point, sourceLocation);
  case MutatorKind::RemoveVoidFunctionMutator:
    return isJunkRemoveVoidFunction(point, sourceLocation);
  case MutatorKind::NegateMutator:
    return isJunkNegateCondition(point, sourceLocation);
  default:
    return false;
  }

  return false;
}

bool CXXJunkDetector::isJunkBoundaryConditional(
    mull::MutationPoint *point, mull::SourceLocation &mutantLocation) {
  auto ast = astStorage.findAST(point);
  auto file = astStorage.findFileEntry(ast, point);

  assert(file);
  assert(file->isValid());
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  ConditionalsBoundaryVisitor visitor(ast->getSourceManager(), location);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  return !visitor.foundMutant();
}

bool CXXJunkDetector::isJunkMathAdd(mull::MutationPoint *point,
                                    mull::SourceLocation &mutantLocation) {
  auto ast = astStorage.findAST(point);
  auto file = astStorage.findFileEntry(ast, point);

  assert(file);
  assert(file->isValid());
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  MathAddVisitor visitor(ast->getSourceManager(), location);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  return !visitor.foundMutant();
}

bool CXXJunkDetector::isJunkMathSub(mull::MutationPoint *point,
                                    mull::SourceLocation &mutantLocation) {
  auto ast = astStorage.findAST(point);
  auto file = astStorage.findFileEntry(ast, point);

  assert(file);
  assert(file->isValid());
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  MathSubVisitor visitor(ast->getSourceManager(), location);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  return !visitor.foundMutant();
}

bool CXXJunkDetector::isJunkRemoveVoidFunction(
    mull::MutationPoint *point, mull::SourceLocation &mutantLocation) {
  auto ast = astStorage.findAST(point);
  auto file = astStorage.findFileEntry(ast, point);

  assert(file);
  assert(file->isValid());
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  RemoveVoidFunctionVisitor visitor(ast->getSourceManager(), location,
                                    ast->getASTContext());
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  return !visitor.foundMutant();
}

bool CXXJunkDetector::isJunkNegateCondition(
    mull::MutationPoint *point, mull::SourceLocation &mutantLocation) {
  auto ast = astStorage.findAST(point);
  auto file = astStorage.findFileEntry(ast, point);

  assert(file);
  assert(file->isValid());
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  NegateConditionVisitor visitor(ast->getSourceManager(), location);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());
  return !visitor.foundMutant();
}
