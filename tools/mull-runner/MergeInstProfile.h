#pragma once

#include <string>
struct MullDiagnostics;
namespace mull {
bool mergeRawInstProfile(const MullDiagnostics &diagnostics, const std::string &input,
                         const std::string &output);
} // namespace mull
