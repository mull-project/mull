#pragma once

#include <string>
#include <vector>

namespace mull {

std::vector<std::string>
findDynamicLibraries(const std::string &executablePath);

}
