#pragma once

#include <string>
#include <vector>

namespace mull {

class Diagnostics;

void resolveLibraries(mull::Diagnostics &diagnostics, std::vector<std::string> &resolvedLibraries,
                      const std::vector<std::string> &libraries,
                      const std::vector<std::string> &librarySearchPaths);

std::vector<std::string> getDynamicLibraryDependencies(mull::Diagnostics &diagnostics,
                                                       const std::string &executablePath);

bool hasCoverage(mull::Diagnostics &diagnostics, const std::string &path);

} // namespace mull
