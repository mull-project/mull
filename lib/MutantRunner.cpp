#include "mull/MutantRunner.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

using namespace mull;

MutantRunner::MutantRunner(Diagnostics &diagnostics, const Configuration &configuration,
                           Runner &runner)
    : diagnostics(diagnostics), configuration(configuration), runner(runner) {}

std::vector<std::unique_ptr<MutationResult>>
MutantRunner::runMutants(const std::string &executable,
                         std::vector<std::unique_ptr<Mutant>> &mutants) {
  return runMutants(executable, {}, mutants);
}

std::vector<std::unique_ptr<MutationResult>>
MutantRunner::runMutants(const std::string &executable, const std::vector<std::string> &extraArgs,
                         std::vector<std::unique_ptr<Mutant>> &mutants) {
  SingleTaskExecutor singleTask(diagnostics);
  ExecutionResult baseline;
  singleTask.execute("Baseline run", [&]() {
    baseline = runner.runProgram(executable,
                                 extraArgs,
                                 {},
                                 configuration.timeout,
                                 configuration.captureMutantOutput,
                                 false,
                                 std::nullopt);
  });

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  std::vector<MutantExecutionTask> tasks;
  tasks.reserve(configuration.parallelization.executionWorkers);
  for (int i = 0; i < configuration.parallelization.executionWorkers; i++) {
    tasks.emplace_back(configuration, diagnostics, executable, baseline, extraArgs);
  }
  TaskExecutor<MutantExecutionTask> mutantRunner(
      diagnostics, "Running mutants", mutants, mutationResults, std::move(tasks));
  mutantRunner.execute();

  return mutationResults;
}
