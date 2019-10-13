#pragma once

#include "mull/MutationPoint.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Testee.h"

namespace mull {

class FilePathFilter;
class Filter;
class Program;
class progress_counter;

class SearchMutationPointsTask {
public:
  using In = const std::vector<MergedTestee>;
  using Out = std::vector<std::unique_ptr<MutationPoint>>;
  using iterator = In::const_iterator;

  SearchMutationPointsTask(Filter &filter, const Program &program,
                           std::vector<std::unique_ptr<Mutator>> &mutators,
                           const FilePathFilter &filePathFilter,
                           const InstructionFilter &instructionFilter);
  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Filter &filter;
  const Program &program;
  std::vector<std::unique_ptr<Mutator>> &mutators;
  const FilePathFilter &filePathFilter;
  const InstructionFilter &instructionFilter;
};

} // namespace mull
