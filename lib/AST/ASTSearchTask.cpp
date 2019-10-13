//
// Created by Stanislav Pankevich on 2019-10-19.
//

#include "mull/AST/ASTSearchTask.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/Parallelization/Progress.h"

using namespace mull;

ASTSearchTask::ASTSearchTask(mull::ASTStorage &astStorage,
                             mull::FilePathFilter &pathFilter,
                             mull::TraverseMask traverseMask)
    : astStorage(astStorage), pathFilter(pathFilter),
      traverseMask(traverseMask) {}

void ASTSearchTask::operator()(iterator begin, iterator end, Out &storage,
                               mull::progress_counter &counter) {
  mull::ASTMutations mutations;

  for (auto it = begin; it != end; it++, counter.increment()) {
    const std::unique_ptr<mull::Bitcode> &bitcode = *it;

    const llvm::Module *const module = bitcode->getModule();

    if (pathFilter.shouldSkip(module->getSourceFileName())) {
      continue;
    }

    /// How can I check that -g flag (debug info enable) was set, from llvm pass
    /// https://stackoverflow.com/a/21713717/598057
    if (module->getNamedMetadata("llvm.dbg.cu") == nullptr) {
      llvm::errs() << "\nerror: ASTFinder: bitcode module does not have debug "
                      "information.\n";
      exit(1);
    }

    const std::string sourceFileName = module->getSourceFileName();
    if (!llvm::sys::fs::is_regular_file(sourceFileName) &&
        sourceFileName != "input.cc") {
      llvm::errs() << "error: ASTFinder: invalid source file path: "
                   << sourceFileName << "\n";
      exit(1);
    }

    auto ast = astStorage.findASTByPath(sourceFileName);

    ASTVisitor visitor(*ast, mutations, pathFilter, traverseMask);
    visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());
  }

  //mutations.dump();

  for (auto elem : mutations.storage) {
    storage.push_back(std::make_pair(elem.first, elem.second));
  }
}
