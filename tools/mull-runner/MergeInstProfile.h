#pragma once

#include <string>

namespace mull {

class Diagnostics;

bool mergeRawInstProfile(Diagnostics &diagnostics, const std::string &input,
                         const std::string &output);

} // namespace mull
