#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

#include "mull/ExecutionResult.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Runner.h"
#include "mull/SourceLocation.h"

#include "rust/mull-core/core.rs.h"

#include <sstream>

using namespace mull;
using namespace std::string_literals;

MutantExecutionTask::MutantExecutionTask(const MullConfig &configuration,
                                         const MullDiagnostics &diagnostics,
                                         const std::string &executable, ExecutionResult &baseline,
                                         const std::vector<std::string> &extraArgs)
    : configuration(configuration), diagnostics(diagnostics), executable(executable),
      baseline(baseline), extraArgs(extraArgs) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  Runner runner(diagnostics);
  std::stringstream debugMessage;
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto &mutant = *it;
    ExecutionResult result;
    if (mutant->isCovered()) {
      result = runner.runProgram(executable,
                                 extraArgs,
                                 { { mutant->getIdentifier(), "1" } },
                                 std::max(30LL, baseline.runningTime * 10),
                                 configuration.capture_mutant_output,
                                 false,
                                 std::nullopt);
    } else {
      result.status = NotCovered;
    }
    storage.push_back(std::make_unique<MutationResult>(result, mutant.get()));
    debugMessage << "Run " << mutant->getIdentifier() << "\n";
    SourceLocation sourceLocation = mutant->getSourceLocation();
    debugMessage << sourceLocation.filePath << ":";
    debugMessage << sourceLocation.line << ":" << sourceLocation.column << " ExecutionResult: ";
    debugMessage << result.getStatusAsString();
    diagnostics.debug(debugMessage.str());
    debugMessage.str(std::string());
  }
}
