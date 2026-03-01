#pragma once

#include <string>
#include <vector>
struct MullDiagnostics;
namespace mull {

void resolveLibraries(const MullDiagnostics &diagnostics,
                      std::vector<std::string> &resolvedLibraries,
                      const std::vector<std::string> &libraries,
                      const std::vector<std::string> &librarySearchPaths);

std::vector<std::string> getDynamicLibraryDependencies(const MullDiagnostics &diagnostics,
                                                       const std::string &executablePath);

bool hasCoverage(const MullDiagnostics &diagnostics, const std::string &path);

} // namespace mull
