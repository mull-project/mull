#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>

namespace mull {

struct VisitorParameters {
  const clang::SourceManager &sourceManager;
  const clang::SourceLocation &sourceLocation;
  clang::ASTContext &astContext;
};

/// Visitors skip the initializer of an implicitly instantiated variable template
template <typename Visitor> void traverseDecl(Visitor &visitor, clang::Decl *decl) {
  visitor.TraverseDecl(decl);
  auto specialization = llvm::dyn_cast<clang::VarTemplateSpecializationDecl>(decl);
  if (specialization &&
      specialization->getTemplateSpecializationKind() != clang::TSK_ExplicitSpecialization) {
    visitor.TraverseStmt(specialization->getInit());
  }
}

} // namespace mull
