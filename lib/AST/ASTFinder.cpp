#include "mull/AST/ASTFinder.h"

#include "mull/AST/ASTSearchTask.h"
#include "mull/AST/ASTVisitor.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/JunkDetection/CXX/Visitors/VisitorParameters.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Program/Program.h"

#include <clang/Basic/SourceLocation.h>
#include <llvm/IR/Module.h>

#include <cassert>
#include <iostream>
#include <regex>

using namespace mull;

ASTFinder::ASTFinder(mull::MutatorKindSet mutatorKindSet) : mutatorKindSet(mutatorKindSet) {}

void mull::ASTFinder::findMutations(mull::Diagnostics &diagnostics, const mull::Configuration &config,
                                    mull::Program &program, mull::FilePathFilter &pathFilter,
                                    mull::ASTStorage &storage) {

  std::vector<std::pair<SourceFilePath, SingleASTUnitMutations>> mutationsAsVector;
  std::vector<mull::ASTSearchTask> tasks;

  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(diagnostics, storage, mutatorKindSet, pathFilter);
  }

  TaskExecutor<mull::ASTSearchTask> finder(diagnostics,
                                           "AST Search: looking for mutations in the source files",
                                           program.bitcode(),
                                           mutationsAsVector,
                                           tasks);
  finder.execute();

  std::unordered_map<SourceFilePath, SingleASTUnitMutations> mutations;

  for (auto &unitMutationsPair: mutationsAsVector) {
    mutations.emplace(unitMutationsPair.first, unitMutationsPair.second);
  }

  storage.saveMutations(mutations);
}
