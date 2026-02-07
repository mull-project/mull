#pragma once

#include "mull/Bitcode.h"
#include <vector>

struct MullConfig;
namespace mull {

class MutationPoint;
class progress_counter;

class CloneMutatedFunctionsTask {
public:
  using In = std::vector<std::unique_ptr<Bitcode>>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  CloneMutatedFunctionsTask() = default;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);
  static void cloneFunctions(Bitcode &bitcode);

private:
};

class DeleteOriginalFunctionsTask {
public:
  using In = std::vector<std::unique_ptr<Bitcode>>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  DeleteOriginalFunctionsTask() = default;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);
  static void deleteFunctions(Bitcode &bitcode);

private:
};

class InsertMutationTrampolinesTask {
public:
  using In = std::vector<std::unique_ptr<Bitcode>>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  explicit InsertMutationTrampolinesTask(const MullConfig &configuration)
      : configuration(configuration) {}

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);
  static void insertTrampolines(Bitcode &bitcode, const MullConfig &configuration);

private:
  const MullConfig &configuration;
};

} // namespace mull
