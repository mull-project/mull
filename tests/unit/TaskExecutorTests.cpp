#include "gtest/gtest.h"

#include "mull/Parallelization/Parallelization.h"

#include <mull/Diagnostics/Diagnostics.h>
#include <vector>

using namespace mull;

class AddNumberTask {
public:
  using In = std::vector<int>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter) {
    for (auto it = begin; it != end; ++it, counter.increment()) {
      storage.push_back((*it) + 1);
    }
  }
};

class EmptyTask {
public:
  using In = std::vector<int>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter) {
    for (auto it = begin; it != end; ++it, counter.increment()) {
    }
  }
};

TEST(TaskExecutor, SequentialExecution_AddNumber_MoreTasks) {
  Diagnostics diagnostics;
  int workers = 1;
  std::vector<AddNumberTask> tasks;
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(AddNumberTask());
  }

  std::vector<int> in({ 1, 2, 3 });
  std::vector<int> out;
  std::vector<int> expected({ 2, 3, 4 });

  TaskExecutor<AddNumberTask> executor(diagnostics, "increment numbers", in, out, std::move(tasks));
  executor.execute();

  ASSERT_EQ(size_t(3), in.size());
  ASSERT_EQ(size_t(3), out.size());

  ASSERT_EQ(expected, out);
}

TEST(TaskExecutor, SequentialExecution_AddNumber_MoreWorkers) {
  Diagnostics diagnostics;
  int workers = 4;
  std::vector<AddNumberTask> tasks;
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(AddNumberTask());
  }

  std::vector<int> in({ 1, 2, 3 });
  std::vector<int> out;
  std::vector<int> expected({ 2, 3, 4 });

  TaskExecutor<AddNumberTask> executor(diagnostics, "increment numbers", in, out, std::move(tasks));
  executor.execute();

  ASSERT_EQ(size_t(3), in.size());
  ASSERT_EQ(size_t(3), out.size());

  ASSERT_EQ(expected, out);
}

TEST(TaskExecutor, SequentialExecution_EmptyTask_MoreTasks) {
  Diagnostics diagnostics;
  int workers = 1;
  std::vector<EmptyTask> tasks;
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(EmptyTask());
  }

  std::vector<int> in({ 1, 2, 3 });
  std::vector<int> out;
  std::vector<int> expected;

  TaskExecutor<EmptyTask> executor(diagnostics, "do nothing", in, out, std::move(tasks));
  executor.execute();

  ASSERT_EQ(size_t(3), in.size());
  ASSERT_EQ(size_t(0), out.size());

  ASSERT_EQ(expected, out);
}

TEST(TaskExecutor, SequentialExecution_EmptyTask_MoreWorkers) {
  Diagnostics diagnostics;
  int workers = 4;
  std::vector<EmptyTask> tasks;
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(EmptyTask());
  }

  std::vector<int> in({ 1, 2, 3 });
  std::vector<int> out;
  std::vector<int> expected;

  TaskExecutor<EmptyTask> executor(diagnostics, "do nothing", in, out, std::move(tasks));
  executor.execute();

  ASSERT_EQ(size_t(3), in.size());
  ASSERT_EQ(size_t(0), out.size());

  ASSERT_EQ(expected, out);
}
