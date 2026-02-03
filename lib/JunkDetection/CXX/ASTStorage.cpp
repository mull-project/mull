#include "mull/JunkDetection/CXX/ASTStorage.h"

#include "mull/MutationPoint.h"
#include "rust/mull-core/core.rs.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#if LLVM_VERSION_MAJOR >= 20
#include <llvm/Support/VirtualFileSystem.h>
#endif
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

ASTUnitWrapper::ASTUnitWrapper(std::unique_ptr<clang::ASTUnit> ast) : ast(std::move(ast)) {
  if (this->ast) {
    recordDeclarations();
  }
}

clang::SourceManager &ASTUnitWrapper::getSourceManager() {
  return ast->getSourceManager();
}

clang::ASTContext &ASTUnitWrapper::getASTContext() {
  return ast->getASTContext();
}

bool ASTUnitWrapper::isInSystemHeader(clang::SourceLocation &location) {
  return ast->getSourceManager().isInSystemHeader(location);
}

const clang::FileEntry *ASTUnitWrapper::findFileEntry(const mull::SourceLocation &sourceLocation) {
  assert(!sourceLocation.isNull() && "Missing debug information?");

  const clang::FileEntry *file = findFileEntry(sourceLocation.filePath);
  return file ? file : findFileEntry(sourceLocation.unitFilePath);
}

const clang::FileEntry *ASTUnitWrapper::findFileEntry(const std::string &filePath) {
  auto &sourceManager = ast->getSourceManager();
  auto begin = sourceManager.fileinfo_begin();
  auto end = sourceManager.fileinfo_end();
  const clang::FileEntry *file = nullptr;
  for (auto it = begin; it != end; it++) {
#if LLVM_VERSION_MAJOR >= 18
    llvm::StringRef currentSourceFilePath = it->first.getName();
#else
    llvm::StringRef currentSourceFilePath = it->first->getName();
#endif

    /// In LLVM 6, it->first->getName() does not expand to full path for header files.
    if (!llvm::sys::path::is_absolute(currentSourceFilePath)) {
#if LLVM_VERSION_MAJOR >= 18
      currentSourceFilePath = it->first.getFileEntry().tryGetRealPathName();
#else
      currentSourceFilePath = it->first->tryGetRealPathName();
#endif
    }
    llvm::SmallString<PATH_MAX> realFilePath;
    llvm::sys::fs::real_path(filePath, realFilePath);
    llvm::SmallString<PATH_MAX> realSourceFilePath;
    llvm::sys::fs::real_path(currentSourceFilePath, realSourceFilePath);
    if (realSourceFilePath.equals(realFilePath)) {
      file = it->first;
      break;
    }
  }

  return file;
}

clang::SourceLocation ASTUnitWrapper::getLocation(const mull::SourceLocation &sourceLocation) {
  auto file = findFileEntry(sourceLocation);
  assert(file);
  assert(!sourceLocation.isNull());
  auto location = ast->getLocation(file, sourceLocation.line, sourceLocation.column);
  assert(location.isValid());
  return location;
}

clang::SourceLocation
ASTUnitWrapper::getLocForEndOfToken(const clang::SourceLocation sourceLocationEnd) {
  clang::SourceManager &sourceManager = ast->getSourceManager();
  /// Clang AST: how to get more precise debug information in certain cases?
  /// http://clang-developers.42468.n3.nabble.com/Clang-AST-how-to-get-more-precise-debug-information-in-certain-cases-td4065195.html
  /// https://stackoverflow.com/questions/11083066/getting-the-source-behind-clangs-ast
  auto sourceLocationEndActual = clang::Lexer::getLocForEndOfToken(
      sourceLocationEnd, 0, sourceManager, ast->getASTContext().getLangOpts());
  return sourceLocationEndActual;
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

void ASTUnitWrapper::recordDeclarations() {
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

clang::Decl *ASTUnitWrapper::getDecl(clang::SourceLocation &location) {
  if (decls.empty()) {
    return nullptr;
  }
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

bool ASTUnitWrapper::hasAST() const {
  return ast != nullptr;
}

ASTStorage::ASTStorage(const MullDiagnostics &diagnostics,
                       const std::string &cxxCompilationDatabasePath,
                       const std::string &cxxCompilationFlags,
                       const std::unordered_map<std::string, std::string> &bitcodeCompilationFlags)
    : diagnostics(diagnostics),
      compilationDatabase(CompilationDatabase::fromFile(
          diagnostics, cxxCompilationDatabasePath, cxxCompilationFlags, bitcodeCompilationFlags)) {}

ASTUnitWrapper *ASTStorage::findAST(const mull::SourceLocation &sourceLocation) {
  const std::string &sourceFile = sourceLocation.unitFilePath;
  if (llvm::sys::fs::exists(sourceFile)) {
    return findAST(sourceFile);
  }

  if (sourceFile == "/in-memory-file.cc") {
    const std::string &unitSourceFile = sourceLocation.filePath;
    return findAST(unitSourceFile);
  }

  diagnostics.warning("ASTUnitWrapper: source location does not exist: " + sourceFile);
  return nullptr;
}

ASTUnitWrapper *ASTStorage::findAST(const std::string &sourceFile) {
  if (astUnits.count(sourceFile)) {
    return astUnits.at(sourceFile).get();
  }

  auto &[compiler, flags] = compilationDatabase.compilationFlagsForFile(sourceFile);
  std::vector<const char *> args({ compiler.c_str() });
  for (auto &flag : flags) {
    args.push_back(flag.c_str());
  }
  if (args.size() == 1) {
    args.push_back(sourceFile.c_str());
  }

  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticsEngine(
      clang::CompilerInstance::createDiagnostics(
#if LLVM_VERSION_MAJOR >= 20
          *llvm::vfs::getRealFileSystem(),
#endif
          new clang::DiagnosticOptions));

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

  auto threadSafeAST = new ASTUnitWrapper(std::unique_ptr<clang::ASTUnit>(std::move(ast)));
  astUnits[sourceFile] = std::unique_ptr<ASTUnitWrapper>(threadSafeAST);
  return threadSafeAST;
}

void ASTStorage::setAST(const std::string &sourceFile, std::unique_ptr<ASTUnitWrapper> astUnit) {
  astUnits[sourceFile] = std::move(astUnit);
}
