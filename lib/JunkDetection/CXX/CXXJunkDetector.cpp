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

template <typename Visitor>
static bool isJunkMutation(ASTStorage &storage, MutationPoint *point,
                           SourceLocation &mutantLocation) {
  auto ast = storage.findAST(point);
  auto file = storage.findFileEntry(ast, point);

  assert(file);
  assert(file->isValid());
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  VisitorParameters parameters = {.sourceManager = ast->getSourceManager(),
                                  .sourceLocation = location,
                                  .astContext = ast->getASTContext()};
  Visitor visitor(parameters);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  return !visitor.foundMutant();
}

CXXJunkDetector::CXXJunkDetector(JunkDetectionConfig &config)
    : astStorage(config.cxxCompDBDirectory, config.cxxCompilationFlags) {}

bool CXXJunkDetector::isJunk(MutationPoint *point) {
  auto mutantLocation = point->getSourceLocation();
  if (mutantLocation.isNull()) {
    return true;
  }

  switch (point->getMutator()->mutatorKind()) {
  case MutatorKind::ConditionalsBoundaryMutator:
    return isJunkMutation<ConditionalsBoundaryVisitor>(astStorage, point,
                                                       mutantLocation);
  case MutatorKind::MathAddMutator:
    return isJunkMutation<MathAddVisitor>(astStorage, point, mutantLocation);
  case MutatorKind::MathSubMutator:
    return isJunkMutation<MathSubVisitor>(astStorage, point, mutantLocation);
  case MutatorKind::RemoveVoidFunctionMutator:
    return isJunkMutation<RemoveVoidFunctionVisitor>(astStorage, point,
                                                     mutantLocation);
  case MutatorKind::NegateMutator:
    return isJunkMutation<NegateConditionVisitor>(astStorage, point,
                                                  mutantLocation);
  default:
    return false;
  }

  return false;
}
