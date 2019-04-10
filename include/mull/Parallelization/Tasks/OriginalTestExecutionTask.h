#pragma once

#include "mull/TestFrameworks/Test.h"
#include "mull/Testee.h"

namespace mull {

class Instrumentation;
class ProcessSandbox;
class TestRunner;
class Filter;
class JITEngine;
class progress_counter;
class Program;

struct Configuration;

class OriginalTestExecutionTask {
public:
  using In = std::vector<Test>;
  using Out = std::vector<std::unique_ptr<Testee>>;
  using iterator = In::iterator;

  OriginalTestExecutionTask(Instrumentation &instrumentation, Program &program,
                            ProcessSandbox &sandbox, TestRunner &runner,
                            const Configuration &config, Filter &filter,
                            JITEngine &jit);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);
  Instrumentation &instrumentation;
  Program &program;
  ProcessSandbox &sandbox;
  TestRunner &runner;
  const Configuration &config;
  Filter &filter;
  JITEngine &jit;
};
} // namespace mull
