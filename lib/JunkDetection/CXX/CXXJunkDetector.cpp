#include "mull/JunkDetection/CXX/CXXJunkDetector.h"

#include "mull/Config/RawConfig.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/Mutator.h"

#include "mull/JunkDetection/CXX/Visitors/AndOrReplacementVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathAddVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathDivVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathMulVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathSubVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/NegateConditionVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/ReplaceCallVisitor.h"

using namespace mull;

template <typename Visitor>
static bool isJunkMutation(ASTStorage &storage, MutationPoint *point) {
  auto ast = storage.findAST(point);
  auto location = ast->getLocation(point);

  if (ast->isInSystemHeader(location)) {
    return true;
  }

  VisitorParameters parameters = {.sourceManager = ast->getSourceManager(),
                                  .sourceLocation = location,
                                  .astContext = ast->getASTContext()};
  Visitor visitor(parameters);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  return !visitor.foundMutant();
}

CXXJunkDetector::CXXJunkDetector(JunkDetectionConfig &config)
    : astStorage(config.cxxCompilationDatabasePath,
                 config.cxxCompilationFlags) {}

bool CXXJunkDetector::isJunk(MutationPoint *point) {
  if (point->getSourceLocation().isNull()) {
    return true;
  }

  switch (point->getMutator()->mutatorKind()) {
  case MutatorKind::ConditionalsBoundaryMutator:
    return isJunkMutation<ConditionalsBoundaryVisitor>(astStorage, point);
  case MutatorKind::MathAddMutator:
    return isJunkMutation<MathAddVisitor>(astStorage, point);
  case MutatorKind::MathSubMutator:
    return isJunkMutation<MathSubVisitor>(astStorage, point);
  case MutatorKind::MathMulMutator:
    return isJunkMutation<MathMulVisitor>(astStorage, point);
  case MutatorKind::MathDivMutator:
    return isJunkMutation<MathDivVisitor>(astStorage, point);
  case MutatorKind::RemoveVoidFunctionMutator:
    return isJunkMutation<RemoveVoidFunctionVisitor>(astStorage, point);
  case MutatorKind::ReplaceCallMutator:
    return isJunkMutation<ReplaceCallVisitor>(astStorage, point);
  case MutatorKind::NegateMutator:
    return isJunkMutation<NegateConditionVisitor>(astStorage, point);
  case MutatorKind::AndOrReplacementMutator:
    return isJunkMutation<AndOrReplacementVisitor>(astStorage, point);
  default:
    return false;
  }

  return false;
}
