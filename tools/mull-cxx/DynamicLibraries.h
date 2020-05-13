#pragma once

#include <string>
#include <vector>

namespace mull {

class Diagnostics;

std::vector<std::string> findDynamicLibraries(mull::Diagnostics &diagnostics,
                                              const std::string &executablePath,
                                              std::vector<std::string> &librarySearchPaths);
} // namespace mull
