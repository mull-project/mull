#pragma once

#include <functional>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <assert.h>

#include "Progress.h"
#include "mull/Metrics/Metrics.h"

namespace mull {

class Diagnostics;

std::vector<int> taskBatches(size_t itemsCount, size_t tasks);
void printTimeSummary(Diagnostics &diagnostics, MetricsMeasure measure);

template <typename Task> class TaskExecutor {
public:
  using In = typename Task::In;
  using Out = typename Task::Out;
  TaskExecutor(Diagnostics &diagnostics, std::string name, In &in, Out &out,
               std::vector<Task> tasks)
      : diagnostics(diagnostics), in(in), out(out), tasks(std::move(tasks)), name(std::move(name)) {
  }

  void execute() {
    if (tasks.empty() || in.empty()) {
      return;
    }

    measure.start();
    if (tasks.size() == 1 || in.size() == 1) {
      executeSequentially();
    } else {
      executeInParallel();
    }
    measure.finish();
    printTimeSummary(diagnostics, measure);
  }

private:
  void executeInParallel() {
    assert(tasks.size() != 1);
    assert(in.size() != 1);
    auto workers = std::min(in.size(), tasks.size());

    auto batches = taskBatches(in.size(), workers);
    std::vector<std::thread> threads;
    std::vector<Out> storages;

    storages.reserve(workers);
    counters.reserve(workers);

    auto end = in.begin();
    for (unsigned i = 0; i < workers; i++) {
      int group = batches[i];
      storages.push_back(Out());
      counters.push_back(progress_counter());

      auto begin = end;
      std::advance(end, group);
      std::thread t(
          std::move(tasks[i]), begin, end, std::ref(storages.back()), std::ref(counters.back()));
      threads.push_back(std::move(t));
    }

    std::thread reporter(progress_reporter{ diagnostics, name, counters, in.size(), workers });
    threads.push_back(std::move(reporter));

    for (auto &t : threads) {
      t.join();
    }

    for (auto &storage : storages) {
      for (auto &m : storage) {
        out.push_back(std::move(m));
      }
    }
  }

  void executeSequentially() {
    assert(tasks.size() == 1 || in.size() == 1);
    auto &task = tasks.front();

    counters.push_back(progress_counter());
    std::thread reporter(progress_reporter{ diagnostics, name, counters, in.size(), 1 });

    task(in.begin(), in.end(), out, std::ref(counters.back()));
    reporter.join();
  }

  Diagnostics &diagnostics;
  In &in;
  Out &out;
  std::vector<Task> tasks;
  std::vector<progress_counter> counters{};
  MetricsMeasure measure;
  std::string name;
};

class SingleTaskTag {};

template <> class TaskExecutor<SingleTaskTag> {
public:
  TaskExecutor(Diagnostics &diagnostics, std::string name, std::function<void(void)> task)
      : diagnostics(diagnostics), name(std::move(name)), task(std::move(task)) {}

  void execute() {
    MetricsMeasure measure;
    measure.start();
    std::vector<progress_counter> unusedCounters{};
    progress_counter::CounterType total(1);
    size_t workers = 1;
    progress_reporter reporter{ diagnostics, name, unusedCounters, total, workers };
    task();
    bool forceReport(true);
    reporter.printProgress(total, total, forceReport);
    measure.finish();
    printTimeSummary(diagnostics, measure);
  }

private:
  Diagnostics &diagnostics;
  std::string name;
  std::function<void(void)> task;
};

typedef TaskExecutor<SingleTaskTag> SingleTaskExecutor;

} // namespace mull
