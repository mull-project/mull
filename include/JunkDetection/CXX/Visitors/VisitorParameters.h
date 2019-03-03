#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>

namespace mull {

struct VisitorParameters {
  const clang::SourceManager &sourceManager;
  const clang::SourceLocation &sourceLocation;
  const clang::ASTContext &astContext;
};

} // namespace mull