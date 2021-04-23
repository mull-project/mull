#include "mull/MutantRunner.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

using namespace mull;

MutantRunner::MutantRunner(Diagnostics &diagnostics, const Configuration &configuration)
    : diagnostics(diagnostics), configuration(configuration), runner(diagnostics) {}

std::vector<std::unique_ptr<MutationResult>>
MutantRunner::runMutants(const std::string &executable,
                         std::vector<std::unique_ptr<Mutant>> &mutants) {
  return runMutants(executable, {}, mutants);
}

std::vector<std::unique_ptr<MutationResult>>
MutantRunner::runMutants(const std::string &executable, const std::vector<std::string> &extraArgs,
                         std::vector<std::unique_ptr<Mutant>> &mutants) {
  SingleTaskExecutor singleTask(diagnostics);
  /// On macOS, sometimes newly compiled programs take more time to execute for the first run
  /// As we take the execution time as a baseline for timeout it makes sense to have an additional
  /// warm up run so that the next runs will be a bit faster
  singleTask.execute("Warm up run", [&]() {
    runner.runProgram(executable,
                      extraArgs,
                      {},
                      configuration.timeout,
                      configuration.captureMutantOutput,
                      std::nullopt);
  });

  ExecutionResult baseline;
  singleTask.execute("Baseline run", [&]() {
    baseline = runner.runProgram(executable,
                                 extraArgs,
                                 {},
                                 configuration.timeout,
                                 configuration.captureMutantOutput,
                                 std::nullopt);
  });

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  std::vector<MutantExecutionTask> tasks;
  tasks.reserve(configuration.parallelization.mutantExecutionWorkers);
  for (int i = 0; i < configuration.parallelization.mutantExecutionWorkers; i++) {
    tasks.emplace_back(configuration, diagnostics, executable, baseline, extraArgs);
  }
  TaskExecutor<MutantExecutionTask> mutantRunner(
      diagnostics, "Running mutants", mutants, mutationResults, std::move(tasks));
  mutantRunner.execute();

  return mutationResults;
}
