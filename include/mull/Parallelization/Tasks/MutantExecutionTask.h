#pragma once

#include "mull/MutationResult.h"
#include "mull/Toolchain/JITEngine.h"

#include <llvm/Object/ObjectFile.h>

namespace mull {

class MutationPoint;
class Driver;
class ProcessSandbox;
class TestRunner;
class Toolchain;
class Mangler;
class progress_counter;
class Program;

struct Configuration;

class MutantExecutionTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  MutantExecutionTask(Diagnostics &diagnostics, const ProcessSandbox &sandbox, Program &program,
                      TestRunner &runner, const Configuration &config, Mangler &mangler,
                      std::vector<llvm::object::ObjectFile *> &objectFiles,
                      std::vector<std::string> &mutatedFunctionNames);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Diagnostics &diagnostics;
  JITEngine jit;
  Program &program;
  const ProcessSandbox &sandbox;
  TestRunner &runner;
  const Configuration &config;
  Mangler &mangler;
  std::vector<llvm::object::ObjectFile *> &objectFiles;
  std::vector<std::string> &mutatedFunctionNames;
};
} // namespace mull
