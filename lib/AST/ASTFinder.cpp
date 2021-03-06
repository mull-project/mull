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

ASTFinder::ASTFinder(const MutatorKindSet &mutatorKindSet) : mutatorKindSet(mutatorKindSet) {}

void ASTFinder::findMutations(Diagnostics &diagnostics, const Configuration &config,
                              Program &program, FilePathFilter &pathFilter, ASTStorage &storage) {

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

  std::unordered_map<SourceFilePath, SingleFileMutations> totalMutations;

  for (auto &unitMutationsPair : mutationsAsVector) {
    for (auto &singleFileMutations : unitMutationsPair.second) {
      if (totalMutations.count(singleFileMutations.first) == 0) {
        totalMutations.emplace(singleFileMutations.first, singleFileMutations.second);
        continue;
      }
      SingleFileMutations &singleFileTotalMutations = totalMutations[singleFileMutations.first];
      for (const auto &oneMutationBucket : singleFileMutations.second) {
        if (singleFileTotalMutations.count(oneMutationBucket.first) == 0) {
          singleFileTotalMutations.emplace(oneMutationBucket.first, oneMutationBucket.second);
          continue;
        }
        singleFileTotalMutations[oneMutationBucket.first].insert(oneMutationBucket.second.begin(),
                                                                 oneMutationBucket.second.end());
      }
      totalMutations.emplace(singleFileMutations.first, singleFileMutations.second);
    }
  }
  storage.saveMutations(totalMutations);
}
