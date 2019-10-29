#pragma once

#include "mull/ReachableFunction.h"
#include "mull/TestFrameworks/Test.h"

namespace mull {

class Instrumentation;
class ProcessSandbox;
class TestRunner;
class JITEngine;
class progress_counter;
class Program;

struct Configuration;

class OriginalTestExecutionTask {
public:
  using In = std::vector<Test>;
  using Out = std::vector<std::unique_ptr<ReachableFunction>>;
  using iterator = In::iterator;

  OriginalTestExecutionTask(Instrumentation &instrumentation, Program &program,
                            const ProcessSandbox &sandbox, TestRunner &runner,
                            const Configuration &config, JITEngine &jit);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Instrumentation &instrumentation;
  Program &program;
  const ProcessSandbox &sandbox;
  TestRunner &runner;
  const Configuration &config;
  JITEngine &jit;
};
} // namespace mull
