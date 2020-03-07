#pragma once

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/FilePathFilter.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

#include <unordered_map>
#include <vector>

namespace mull {

class progress_counter;

class ASTSearchTask {
public:
  using In = const std::vector<std::unique_ptr<mull::Bitcode>>;
  using Out = std::vector<std::pair<SourceFilePath, SingleASTUnitMutations>>;
  using iterator = In::const_iterator;

  ASTSearchTask(mull::Diagnostics &diagnostics, mull::ASTStorage &astStorage,
                mull::MutatorKindSet mutatorKindSet, mull::FilePathFilter &pathFilter);

  void operator()(iterator begin, iterator end, Out &storage, mull::progress_counter &counter);

private:
  mull::Diagnostics &diagnostics;
  mull::ASTStorage &astStorage;
  mull::MutatorKindSet mutatorKindSet;
  mull::FilePathFilter &pathFilter;
};

} // namespace mull
