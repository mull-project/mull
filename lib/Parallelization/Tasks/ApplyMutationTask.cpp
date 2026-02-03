#include "mull/Parallelization/Tasks/ApplyMutationTask.h"

#include "mull/MutationPoint.h"
#include "mull/Parallelization/Progress.h"
#include "rust/mull-core/core.rs.h"
#include <llvm/IR/Verifier.h>
#include <sstream>

using namespace mull;

ApplyMutationTask::ApplyMutationTask(const MullConfig &config, const MullDiagnostics &diagnostics)
    : config(config), diagnostics(diagnostics) {}

void ApplyMutationTask::operator()(iterator begin, iterator end, Out &storage,
                                   progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto point = *it;
    point->recordMutation();
    point->applyMutation();
    if (config.debug.slow_ir_verification) {
      auto module = point->getBitcode()->getModule();
      std::string error;
      llvm::raw_string_ostream errorStream(error);
      if (llvm::verifyModule(*module, &errorStream)) {
        std::stringstream message;
        std::string mutator = point->getMutatorIdentifier();
        message << "Uh oh! Mull corrupted LLVM module.\n"
                << "Please, report the following error message here "
                   "https://github.com/mull-project/mull/issues\n"
                << "Underlying error message:\n"
                << errorStream.str() << "\n"
                << "The module was corrupted by '" << mutator << "' mutator.\n"
                << "To disable it, add the following:\n"
                   "ignoreMutators:\n"
                << "  - " << mutator << "\n"
                << "to the config file";
        if (!config.config_path.empty()) {
          message << " (" << std::string(config.config_path) << ")";
        }
        message << "\n";
        diagnostics.error(message.str());
      }
    }
  }
}
