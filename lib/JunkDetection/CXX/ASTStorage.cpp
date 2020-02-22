#include "mull/JunkDetection/CXX/ASTStorage.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/MutationPoint.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <sstream>

using namespace mull;
using namespace llvm;

class DeclVisitor : public clang::RecursiveASTVisitor<DeclVisitor> {

public:
  DeclVisitor(clang::SourceManager &sourceManager, std::vector<clang::Decl *> &decls)
      : sourceManager(sourceManager), declarations(decls) {}

  bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *decl) {
    addDecl(decl);
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *decl) {
    addDecl(decl);
    return true;
  }

private:
  void addDecl(clang::Decl *decl) {
    if (!decl->hasBody()) {
      return;
    }
    if (sourceManager.isInSystemHeader(decl->getSourceRange().getBegin())) {
      return;
    }
    declarations.push_back(decl);
  }

  clang::SourceManager &sourceManager;
  std::vector<clang::Decl *> &declarations;
};

ThreadSafeASTUnit::ThreadSafeASTUnit(clang::ASTUnit *ast)
    : ast(std::unique_ptr<clang::ASTUnit>(ast)) {
  if (this->ast) {
    recordDeclarations();
  }
}

clang::SourceManager &ThreadSafeASTUnit::getSourceManager() {
  return ast->getSourceManager();
}

clang::ASTContext &ThreadSafeASTUnit::getASTContext() {
  return ast->getASTContext();
}

bool ThreadSafeASTUnit::isInSystemHeader(clang::SourceLocation &location) {
  return ast->getSourceManager().isInSystemHeader(location);
}

const clang::FileEntry *ThreadSafeASTUnit::findFileEntry(const MutationPoint *point) {
  assert(point);

  SourceLocation sourceLocation = point->getSourceLocation();
  assert(!sourceLocation.isNull() && "Missing debug information?");

  const clang::FileEntry *file = findFileEntry(sourceLocation.filePath);
  return file ? file : findFileEntry(sourceLocation.unitFilePath);
}

const clang::FileEntry *ThreadSafeASTUnit::findFileEntry(const std::string &filePath) {
  auto &sourceManager = ast->getSourceManager();
  auto begin = sourceManager.fileinfo_begin();
  auto end = sourceManager.fileinfo_end();
  const clang::FileEntry *file = nullptr;
  for (auto it = begin; it != end; it++) {
    StringRef name(it->first->getName());
    if (name.equals(filePath)) {
      file = it->first;
      break;
    }
  }

  return file;
}

clang::SourceLocation ThreadSafeASTUnit::getLocation(MutationPoint *point) {
  auto file = findFileEntry(point);
  assert(file);
  assert(file->isValid());

  auto mutantLocation = point->getSourceLocation();
  assert(!mutantLocation.isNull());

  /// getLocation from the ASTUnit it not thread safe
  std::lock_guard<std::mutex> lock(mutex);
  auto location = ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  return location;
}

struct SortLocationComparator {
  explicit SortLocationComparator(clang::SourceManager &sourceManager)
      : sourceManager(sourceManager), cmp(sourceManager) {}

  bool operator()(const clang::Decl *lhs, const clang::Decl *rhs) const {
    return cmp(lhs->getSourceRange().getBegin(), rhs->getSourceRange().getBegin());
  }

  clang::SourceManager &sourceManager;
  clang::BeforeThanCompare<clang::SourceLocation> cmp;
};

struct UniqueLocationComparator {
  explicit UniqueLocationComparator(clang::SourceManager &sourceManager)
      : sourceManager(sourceManager), cmp(sourceManager) {}

  bool operator()(const clang::Decl *lhs, const clang::Decl *rhs) const {
    return !cmp(lhs->getSourceRange().getEnd(), rhs->getSourceRange().getBegin());
  }

  clang::SourceManager &sourceManager;
  clang::BeforeThanCompare<clang::SourceLocation> cmp;
};

void ThreadSafeASTUnit::recordDeclarations() {
  assert(ast);
  clang::SourceManager &sourceManager = ast->getSourceManager();
  DeclVisitor visitor(sourceManager, decls);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  SortLocationComparator sortComparator(sourceManager);
  std::sort(decls.begin(), decls.end(), sortComparator);

  UniqueLocationComparator uniqueComparator(sourceManager);
  auto last = std::unique(decls.begin(), decls.end(), uniqueComparator);
  decls.erase(last, decls.end());
}

clang::Decl *ThreadSafeASTUnit::getDecl(clang::SourceLocation &location) {
  if (decls.empty()) {
    return nullptr;
  }
  std::lock_guard<std::mutex> lock(mutex);
  clang::BeforeThanCompare<clang::SourceLocation> comparator(ast->getSourceManager());

  auto lower = std::lower_bound(decls.begin(),
                                decls.end(),
                                location,
                                [&](const clang::Decl *decl, const clang::SourceLocation &loc) {
                                  return comparator(decl->getSourceRange().getEnd(), loc);
                                });

  if (lower == decls.end()) {
    return nullptr;
  }

  if (comparator((*lower)->getSourceRange().getBegin(), location) &&
      comparator(location, (*lower)->getSourceRange().getEnd())) {
    return *lower;
  }

  return nullptr;
}

ASTStorage::ASTStorage(Diagnostics &diagnostics, const std::string &cxxCompilationDatabasePath,
                       const std::string &cxxCompilationFlags)
    : diagnostics(diagnostics),
      compilationDatabase(diagnostics, CompilationDatabase::Path(cxxCompilationDatabasePath),
                          CompilationDatabase::Flags(cxxCompilationFlags)) {}

ThreadSafeASTUnit *ASTStorage::findAST(const MutationPoint *point) {
  assert(point);
  assert(!point->getSourceLocation().isNull() && "Missing debug information?");

  const std::string &sourceFile = point->getSourceLocation().unitFilePath;

  std::lock_guard<std::mutex> guard(mutex);
  if (astUnits.count(sourceFile)) {
    return astUnits.at(sourceFile).get();
  }

  auto compilationFlags = compilationDatabase.compilationFlagsForFile(sourceFile);
  std::vector<const char *> args({ "mull-cxx" });
  for (auto &flag : compilationFlags) {
    args.push_back(flag.c_str());
  }
  args.push_back(sourceFile.c_str());

  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticsEngine(
      clang::CompilerInstance::createDiagnostics(new clang::DiagnosticOptions));

  auto ast = clang::ASTUnit::LoadFromCommandLine(args.data(),
                                                 args.data() + args.size(),
                                                 std::make_shared<clang::PCHContainerOperations>(),
                                                 diagnosticsEngine,
                                                 "");

  bool hasErrors = (ast == nullptr) || diagnosticsEngine->hasErrorOccurred() ||
                   diagnosticsEngine->hasUnrecoverableErrorOccurred() ||
                   diagnosticsEngine->hasUncompilableErrorOccurred() ||
                   diagnosticsEngine->hasFatalErrorOccurred();
  if (hasErrors) {
    std::stringstream message;
    message << "Cannot parse file: '" << sourceFile << "':\n";
    for (auto &arg : args) {
      message << arg << " ";
    }
    message << "\n";
    message << "Make sure that the flags provided to Mull are the same flags "
               "that are used for normal compilation.";
    diagnostics.warning(message.str());
  }

  auto threadSafeAST = new ThreadSafeASTUnit(ast);
  astUnits[sourceFile] = std::unique_ptr<ThreadSafeASTUnit>(threadSafeAST);
  return threadSafeAST;
}

clang::Expr *ASTStorage::getMutantASTNode(MutationPoint *mutationPoint) {
  std::lock_guard<std::mutex> lockGuard(mutantNodesMutex);
  if (mutantASTNodes.count(mutationPoint) == 0) {
    return nullptr;
  }

  clang::Expr *mutantASTNode = mutantASTNodes[mutationPoint];
  return mutantASTNode;
}

void ASTStorage::setMutantASTNode(MutationPoint *mutationPoint, clang::Expr *mutantExpression) {
  std::lock_guard<std::mutex> lockGuard(mutantNodesMutex);
  mutantASTNodes[mutationPoint] = mutantExpression;
}
