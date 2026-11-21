// clang-format off
#include "unwrap.h"

struct narrow {
  int value;
  operator unsigned char() const {
    return value;
  }
};

long long unwrap_narrow(int n) {
  return unwrap(narrow{ n });
  // CHECK-DAG: narrow.cpp:[[@LINE-1]]:10: warning: Killed: Replaced unwrap(narrow{ n }) with ((void)(narrow{ n }), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: narrow.cpp:[[@LINE-2]]:10: warning: Killed: Replaced unwrap(narrow{ n }) with ((void)(narrow{ n }), 0) [cxx_replace_scalar_call_zr]
}
