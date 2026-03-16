#include "mull/Filters/Filters.h"

#include "mull/Filters/FilePathFilter.h"

#include "rust/mull-cxx-bridge/bridge.rs.h"

#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace mull;
using namespace std::string_literals;

Filters::Filters(const MullConfig &configuration, const MullDiagnostics &diagnostics)
    : mutationFilters(), configuration(configuration), diagnostics(diagnostics) {}

void Filters::enableFilePathFilter() {
  if (configuration.include_paths.empty() && configuration.exclude_paths.empty()) {
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: both includePaths and excludePaths are empty");
    }
    return;
  }

  auto *filter = new mull::FilePathFilter;
  storage.emplace_back(filter);
  mutationFilters.push_back(filter);

  for (const auto &regex : configuration.exclude_paths) {
    auto std_regex = std::string(regex);
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: excluding: "s + std_regex);
    }
    auto added = filter->exclude(std_regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for exclude-path: '" << std_regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
  for (const auto &regex : configuration.include_paths) {
    auto std_regex = std::string(regex);
    if (configuration.debug.filters) {
      diagnostics.debug("FilePath: including: "s + std_regex);
    }
    auto added = filter->include(std_regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for include-path: '" << std_regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
}
