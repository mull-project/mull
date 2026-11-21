// clang-format off
#pragma once

// Instantiated in two translation units, each conversion returns a different type: no source
// equivalent, no cast of the original fits both
template <typename W> long long unwrap(W w) {
  const auto u = w + 0u;
  // IR-DAG: unwrap.h:[[@LINE-1]]:18: warning: Killed: Replaced w with ((void)(w), 42) [cxx_replace_scalar_call_nz]
  // IR-DAG: unwrap.h:[[@LINE-2]]:18: warning: Killed: Replaced w with ((void)(w), 0) [cxx_replace_scalar_call_zr]
  return u + sizeof(u);
}
