#include "JunkDetection/CXX/ASTStorage.h"
#include "MutationPoint.h"
#include "Logger.h"

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

ASTStorage::ASTStorage(const std::string &cxxCompilationDatabasePath,
                       const std::string &cxxCompilationFlags)
    : compilationDatabase(CompilationDatabase::Path(cxxCompilationDatabasePath),
                          CompilationDatabase::Flags(cxxCompilationFlags)) {}

const clang::ASTUnit *ASTStorage::findAST(const MutationPoint *point) {
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

  auto compilationFlags = compilationDatabase.compilationFlagsForFile(sourceFile);
  std::vector<const char *> args({"mull-cxx"});
  for (auto &flag : compilationFlags) {
    args.push_back(flag.c_str());
  }
  args.push_back(sourceFile.c_str());

  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticsEngine(
      clang::CompilerInstance::createDiagnostics(new clang::DiagnosticOptions));

  auto ast = clang::ASTUnit::LoadFromCommandLine(
      args.data(), args.data() + args.size(),
      std::make_shared<clang::PCHContainerOperations>(), diagnosticsEngine, "");
  astUnits[sourceFile] = std::move(std::unique_ptr<clang::ASTUnit>(ast));
  return ast;
}

const clang::FileEntry *
ASTStorage::findFileEntry(const clang::ASTUnit *ast,
                               const MutationPoint *point) {
  assert(ast);
  assert(point);
  assert(!point->getSourceLocation().isNull());

  auto &sourceManager = ast->getSourceManager();
  auto filePath = point->getSourceLocation().filePath;
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
