#pragma once

#include "mull/Config/Configuration.h"
#include "mull/Core/MutationResult.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Runner/Runner.h"

#include <memory>
#include <vector>

namespace mull {

class MutantRunner {
public:
  MutantRunner(Diagnostics &diagnostics, const Configuration &configuration, Runner &runner);
  std::vector<std::unique_ptr<MutationResult>>
  runMutants(const std::string &executable, std::vector<std::unique_ptr<Mutant>> &mutants);
  std::vector<std::unique_ptr<MutationResult>>
  runMutants(const std::string &executable, const std::vector<std::string> &extraArgs,
             std::vector<std::unique_ptr<Mutant>> &mutants);

private:
  Diagnostics &diagnostics;
  const Configuration &configuration;
  Runner &runner;
};

} // namespace mull
