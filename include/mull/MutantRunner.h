#pragma once

#include "Runner.h"
#include "mull/MutationResult.h"

#include <memory>
#include <vector>

struct MullConfig;
struct MullDiagnostics;

namespace mull {

class MutantRunner {
public:
  MutantRunner(const MullDiagnostics &diagnostics, const MullConfig &configuration, Runner &runner);
  std::vector<std::unique_ptr<MutationResult>>
  runMutants(const std::string &executable, std::vector<std::unique_ptr<Mutant>> &mutants);
  std::vector<std::unique_ptr<MutationResult>>
  runMutants(const std::string &executable, const std::vector<std::string> &extraArgs,
             std::vector<std::unique_ptr<Mutant>> &mutants);

private:
  const MullDiagnostics &diagnostics;
  const MullConfig &configuration;
  Runner &runner;
};

} // namespace mull
