//
// Created by Stanislav Pankevich on 2019-10-13.
//

#include "mull/AST/ASTFinder.h"

#include "mull/AST/ASTMutations.h"
#include "mull/AST/ASTSearchTask.h"
#include "mull/AST/ASTVisitor.h"
#include "mull/Config/Configuration.h"
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

mull::ASTMutations mull::ASTFinder::findMutations(
    const mull::Configuration &config, mull::Program &program,
    mull::FilePathFilter &pathFilter, mull::ASTStorage &storage,
    mull::TraverseMask traverseMask) {

  std::vector<std::pair<SourceFilePath, SingleASTUnitMutations>>
      mutationsAsVector;
  //  std::unordered_map<in t, int> hello;
  std::vector<mull::ASTSearchTask> tasks;

  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(storage, pathFilter, traverseMask);
  }

  TaskExecutor<mull::ASTSearchTask> finder(
      "AST Search: looking for mutations in the source files",
      program.bitcode(), mutationsAsVector, tasks);
  finder.execute();

  mull::ASTMutations mutations;

  for (auto &mutationPair : mutationsAsVector) {
    mutations.storage.emplace(mutationPair.first, mutationPair.second);
  }

  llvm::errs() << "AST Search: found " << mutations.count() << " mutations"
               << "\n";
  return mutations;
}
