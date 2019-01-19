#pragma once

#include "TestFrameworks/Test.h"
#include "Testee.h"

namespace mull {

class Instrumentation;
class ProcessSandbox;
class TestRunner;
class Filter;
class JITEngine;
class progress_counter;

struct Configuration;

class OriginalTestExecutionTask {
public:
  using In = std::vector<std::unique_ptr<Test>>;
  using Out = std::vector<std::unique_ptr<Testee>>;
  using iterator = In::const_iterator;

  OriginalTestExecutionTask(Instrumentation &instrumentation,
                            ProcessSandbox &sandbox,
                            TestRunner &runner,
                            const Configuration &config,
                            Filter &filter,
                            JITEngine &jit);

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter);
  Instrumentation &instrumentation;
  ProcessSandbox &sandbox;
  TestRunner &runner;
  const Configuration &config;
  Filter &filter;
  JITEngine &jit;
};
}
