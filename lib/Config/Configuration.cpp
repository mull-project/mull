#include "mull/Config/Configuration.h"

namespace mull {

int MullDefaultTimeoutMilliseconds = 3000;

Configuration::Configuration()
    : pathOnDisk(), captureMutantOutput(true), timeout(MullDefaultTimeoutMilliseconds) {
  this->parallelization = ParallelizationConfig::defaultConfig();
}

} // namespace mull
