#include "mull/JunkDetection/CXX/CXXJunkDetector.h"

#include "mull/MutationPoint.h"
#include "mull/Mutators/Mutator.h"

#include "mull/JunkDetection/CXX/Visitors/BinaryVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/NegateConditionVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/ReplaceCallVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/ScalarValueVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/UnaryVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/VarDeclVisitor.h"

using namespace mull;

template <typename Visitor>
static bool isJunkMutation(ASTStorage &storage,
                           MutationPoint *point) {
  auto ast = storage.findAST(point);
  auto location = ast->getLocation(point);
  clang::SourceManager &sourceManager = ast->getSourceManager();

  if (ast->isInSystemHeader(location)) {
    return true;
  }

  clang::Decl *decl = ast->getDecl(location);
  if (!decl) {
    return true;
  }

  VisitorParameters parameters = { .sourceManager = sourceManager,
                                   .sourceLocation = location,
                                   .astContext = ast->getASTContext() };
  Visitor visitor(parameters);
  visitor.TraverseDecl(decl);

  if (clang::Expr *mutantExpression = visitor.foundMutant()) {
    const std::string &sourceFile = point->getSourceLocation().unitFilePath;

    int beginLine = sourceManager.getExpansionLineNumber(location, nullptr);
    int beginColumn = sourceManager.getExpansionColumnNumber(location);

    storage.saveMutation(
        sourceFile, point->getMutator()->mutatorKind(), mutantExpression, beginLine, beginColumn);
    return false;
  }

  return true;
}

CXXJunkDetector::CXXJunkDetector(ASTStorage &astStorage)
    : astStorage(astStorage) {}

bool CXXJunkDetector::isJunk(MutationPoint *point) {
  if (point->getSourceLocation().isNull()) {
    return true;
  }

  switch (point->getMutator()->mutatorKind()) {
  case MutatorKind::RemoveVoidFunctionMutator:
    return isJunkMutation<RemoveVoidFunctionVisitor>(astStorage, point);
  case MutatorKind::ReplaceCallMutator:
    return isJunkMutation<ReplaceCallVisitor>(astStorage, point);
  case MutatorKind::NegateMutator:
    return isJunkMutation<NegateConditionVisitor>(astStorage, point);
  case MutatorKind::ScalarValueMutator:
    return isJunkMutation<ScalarValueVisitor>(astStorage, point);

  case MutatorKind::CXX_LessThanToLessOrEqual:
    return isJunkMutation<cxx::LessThanVisitor>(astStorage, point);
  case MutatorKind::CXX_LessOrEqualToLessThan:
    return isJunkMutation<cxx::LessOrEqualVisitor>(astStorage, point);
  case MutatorKind::CXX_GreaterThanToGreaterOrEqual:
    return isJunkMutation<cxx::GreaterThanVisitor>(astStorage, point);
  case MutatorKind::CXX_GreaterOrEqualToGreaterThan:
    return isJunkMutation<cxx::GreaterOrEqualVisitor>(astStorage, point);
  case MutatorKind::CXX_EqualToNotEqual:
    return isJunkMutation<cxx::EqualVisitor>(astStorage, point);
  case MutatorKind::CXX_NotEqualToEqual:
    return isJunkMutation<cxx::NotEqualVisitor>(astStorage, point);
  case MutatorKind::CXX_GreaterThanToLessOrEqual:
    return isJunkMutation<cxx::GreaterThanVisitor>(astStorage, point);
  case MutatorKind::CXX_GreaterOrEqualToLessThan:
    return isJunkMutation<cxx::GreaterOrEqualVisitor>(astStorage, point);
  case MutatorKind::CXX_LessThanToGreaterOrEqual:
    return isJunkMutation<cxx::LessThanVisitor>(astStorage, point);
  case MutatorKind::CXX_LessOrEqualToGreaterThan:
    return isJunkMutation<cxx::LessOrEqualVisitor>(astStorage, point);

  case MutatorKind::CXX_AddToSub:
    return isJunkMutation<cxx::AddVisitor>(astStorage, point);
  case MutatorKind::CXX_AddAssignToSubAssign:
    return isJunkMutation<cxx::AddAssignVisitor>(astStorage, point);
  case MutatorKind::CXX_PreIncToPreDec:
    return isJunkMutation<cxx::PreIncVisitor>(astStorage, point);
  case MutatorKind::CXX_PostIncToPostDec:
    return isJunkMutation<cxx::PostIncVisitor>(astStorage, point);

  case MutatorKind::CXX_SubToAdd:
    return isJunkMutation<cxx::SubVisitor>(astStorage, point);
  case MutatorKind::CXX_SubAssignToAddAssign:
    return isJunkMutation<cxx::SubAssignVisitor>(astStorage, point);
  case MutatorKind::CXX_PreDecToPreInc:
    return isJunkMutation<cxx::PreDecVisitor>(astStorage, point);
  case MutatorKind::CXX_PostDecToPostInc:
    return isJunkMutation<cxx::PostDecVisitor>(astStorage, point);

  case MutatorKind::CXX_MulToDiv:
    return isJunkMutation<cxx::MulVisitor>(astStorage, point);
  case MutatorKind::CXX_MulAssignToDivAssign:
    return isJunkMutation<cxx::MulAssignVisitor>(astStorage, point);

  case MutatorKind::CXX_DivToMul:
    return isJunkMutation<cxx::DivVisitor>(astStorage, point);
  case MutatorKind::CXX_DivAssignToMulAssign:
    return isJunkMutation<cxx::DivAssignVisitor>(astStorage, point);

  case MutatorKind::CXX_RemToDiv:
    return isJunkMutation<cxx::RemVisitor>(astStorage, point);
  case MutatorKind::CXX_RemAssignToDivAssign:
    return isJunkMutation<cxx::RemAssignVisitor>(astStorage, point);

  case MutatorKind::CXX_BitwiseNotToNoop:
    return isJunkMutation<cxx::BitwiseNotVisitor>(astStorage, point);

  case MutatorKind::CXX_UnaryMinusToNoop:
    return isJunkMutation<cxx::UnaryMinusVisitor>(astStorage, point);

  case MutatorKind::CXX_LShiftToRShift:
    return isJunkMutation<cxx::LShiftVisitor>(astStorage, point);
  case MutatorKind::CXX_LShiftAssignToRShiftAssign:
    return isJunkMutation<cxx::LShiftAssignVisitor>(astStorage, point);
  case MutatorKind::CXX_RShiftToLShift:
    return isJunkMutation<cxx::RShiftVisitor>(astStorage, point);
  case MutatorKind::CXX_RShiftAssignToLShiftAssign:
    return isJunkMutation<cxx::RShiftAssignVisitor>(astStorage, point);

  case MutatorKind::CXX_Bitwise_AndToOr:
    return isJunkMutation<cxx::BitAndVisitor>(astStorage, point);
  case MutatorKind::CXX_Bitwise_AndAssignToOrAssign:
    return isJunkMutation<cxx::BitAndAssignVisitor>(astStorage, point);
  case MutatorKind::CXX_Bitwise_OrToAnd:
    return isJunkMutation<cxx::BitOrVisitor>(astStorage, point);
  case MutatorKind::CXX_Bitwise_OrAssignToAndAssign:
    return isJunkMutation<cxx::BitOrAssignVisitor>(astStorage, point);
  case MutatorKind::CXX_Bitwise_XorToOr:
    return isJunkMutation<cxx::BitXorVisitor>(astStorage, point);
  case MutatorKind::CXX_Bitwise_XorAssignToOrAssign:
    return isJunkMutation<cxx::BitXorAssignVisitor>(astStorage, point);

  case MutatorKind::CXX_AssignConst:
    return isJunkMutation<cxx::AssignVisitor>(astStorage, point);
  case MutatorKind::CXX_InitConst:
    return isJunkMutation<cxx::VarDeclVisitor>(astStorage, point);

  case MutatorKind::CXX_Logical_AndToOr:
    return isJunkMutation<cxx::LogicalAndVisitor>(astStorage, point);
  case MutatorKind::CXX_Logical_OrToAnd:
    return isJunkMutation<cxx::LogicalOrVisitor>(astStorage, point);

  default:
    return true;
  }
}
