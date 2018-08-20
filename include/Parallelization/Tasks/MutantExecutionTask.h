#pragma once

#include "MutationResult.h"
#include "Toolchain/JITEngine.h"

namespace mull {

class MutationPoint;
class Driver;
class ProcessSandbox;
class TestRunner;
class Config;
class Toolchain;
class Filter;
class progress_counter;

class MutantExecutionTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  MutantExecutionTask(Driver &driver,
                      ProcessSandbox &sandbox,
                      TestRunner &runner,
                      Config &config,
                      Toolchain &toolchain,
                      Filter &filter);

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter);
  JITEngine jit;
  ProcessSandbox &sandbox;
  TestRunner &runner;
  Config &config;
  Toolchain &toolchain;
  Filter &filter;
  Driver &driver;
};
}
