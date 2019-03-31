#include "mull/JunkDetection/CXX/ASTStorage.h"

#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <clang/Frontend/CompilerInstance.h>

using namespace mull;
using namespace llvm;

ThreadSafeASTUnit::ThreadSafeASTUnit(clang::ASTUnit *ast)
    : ast(std::unique_ptr<clang::ASTUnit>(ast)) {}

clang::SourceManager &ThreadSafeASTUnit::getSourceManager() {
  return ast->getSourceManager();
}

clang::ASTContext &ThreadSafeASTUnit::getASTContext() {
  return ast->getASTContext();
}

bool ThreadSafeASTUnit::isInSystemHeader(clang::SourceLocation &location) {
  return ast->getSourceManager().isInSystemHeader(location);
}

const clang::FileEntry *
ThreadSafeASTUnit::findFileEntry(const MutationPoint *point) {
  assert(point);
  assert(!point->getSourceLocation().isNull());

  auto filePath = point->getSourceLocation().filePath;
  const clang::FileEntry *file = findFileEntry(filePath);
  if (file != nullptr) {
    return file;
  }

  auto instruction = dyn_cast<Instruction>(point->getOriginalValue());
  if (instruction == nullptr) {
    return nullptr;
  }

  const std::string &sourceFile = instruction->getModule()->getSourceFileName();

  return findFileEntry(sourceFile);
}

const clang::FileEntry *
ThreadSafeASTUnit::findFileEntry(const std::string &filePath) {
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
  auto location =
      ast->getLocation(file, mutantLocation.line, mutantLocation.column);
  assert(location.isValid());
  return location;
}

ASTStorage::ASTStorage(const std::string &cxxCompilationDatabasePath,
                       const std::string &cxxCompilationFlags)
    : compilationDatabase(CompilationDatabase::Path(cxxCompilationDatabasePath),
                          CompilationDatabase::Flags(cxxCompilationFlags)) {}

ThreadSafeASTUnit *ASTStorage::findAST(const MutationPoint *point) {
  assert(point);
  assert(!point->getSourceLocation().isNull());

  auto instruction = dyn_cast<Instruction>(point->getOriginalValue());
  if (instruction == nullptr) {
    return nullptr;
  }

  const std::string &sourceFile = instruction->getModule()->getSourceFileName();
  std::lock_guard<std::mutex> guard(mutex);
  if (astUnits.count(sourceFile)) {
    return astUnits.at(sourceFile).get();
  }

  auto compilationFlags =
      compilationDatabase.compilationFlagsForFile(sourceFile);
  std::vector<const char *> args({"mull-cxx"});
  for (auto &flag : compilationFlags) {
    args.push_back(flag.c_str());
  }
  args.push_back(sourceFile.c_str());

  //  for (auto &x : args) {
  //    printf("%s ", x);
  //  }
  //  printf("\n");

  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticsEngine(
      clang::CompilerInstance::createDiagnostics(new clang::DiagnosticOptions));

  auto ast = clang::ASTUnit::LoadFromCommandLine(
      args.data(), args.data() + args.size(),
      std::make_shared<clang::PCHContainerOperations>(), diagnosticsEngine, "");

  auto threadSafeAST = new ThreadSafeASTUnit(ast);
  astUnits[sourceFile] = std::unique_ptr<ThreadSafeASTUnit>(threadSafeAST);
  return threadSafeAST;
}
