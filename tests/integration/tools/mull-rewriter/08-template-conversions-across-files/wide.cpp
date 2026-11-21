// clang-format off
#include "unwrap.h"

struct wide {
  int value;
  operator long long() const {
    return value;
  }
};

long long unwrap_wide(int n) {
  return unwrap(wide{ n * 1000 });
  // CHECK-DAG: wide.cpp:[[@LINE-1]]:10: warning: Killed: Replaced unwrap(wide{ n * 1000 }) with ((void)(wide{ n * 1000 }), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: wide.cpp:[[@LINE-2]]:10: warning: Killed: Replaced unwrap(wide{ n * 1000 }) with ((void)(wide{ n * 1000 }), 0) [cxx_replace_scalar_call_zr]
}
