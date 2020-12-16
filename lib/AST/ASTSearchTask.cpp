#include "mull/AST/ASTSearchTask.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/Parallelization/Progress.h"
#include <llvm/Support/FileSystem.h>

using namespace mull;

ASTSearchTask::ASTSearchTask(mull::Diagnostics &diagnostics, mull::ASTStorage &astStorage,
                             mull::MutatorKindSet mutatorKindSet, mull::FilePathFilter &pathFilter)
    : diagnostics(diagnostics), astStorage(astStorage), mutatorKindSet(mutatorKindSet),
      pathFilter(pathFilter) {}

void ASTSearchTask::operator()(iterator begin, iterator end, Out &storage,
                               mull::progress_counter &counter) {

  std::vector<std::pair<SourceFilePath, SingleASTUnitMutations>> mutationUnits;

  for (auto it = begin; it != end; it++, counter.increment()) {
    const std::unique_ptr<mull::Bitcode> &bitcode = *it;

    const llvm::Module *const module = bitcode->getModule();
    std::string sourceFileName = module->getSourceFileName();
    if (pathFilter.shouldSkip(sourceFileName)) {
      diagnostics.debug(std::string("ASTSearchTask: skipping path: ") + sourceFileName);
      continue;
    }

    if (!llvm::sys::fs::is_regular_file(sourceFileName) && sourceFileName != "/in-memory-file.cc") {
      diagnostics.error(std::string("ASTSearchTask: invalid source file path: ") + sourceFileName);
    }

    auto ast = astStorage.findAST(sourceFileName);

    mutationUnits.push_back(std::make_pair(sourceFileName, SingleASTUnitMutations()));
    SingleASTUnitMutations &singleUnitMutations = mutationUnits.back().second;

    ASTVisitor visitor(diagnostics, *ast, singleUnitMutations, pathFilter, mutatorKindSet);
    visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());
  }

  for (auto &elem : mutationUnits) {
    storage.push_back(std::make_pair(elem.first, elem.second));
  }
}
