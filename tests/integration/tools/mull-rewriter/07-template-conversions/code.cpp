// clang-format off
struct narrow {
  int value;
  operator unsigned char() const {
    return value;
  }
};

struct wide {
  int value;
  operator long long() const {
    return value;
  }
};

// No source equivalent: the conversion returns a different type in each instantiation, a cast of
// the original to one of them truncates the other, '__typeof__(w)' is the class
template <typename W> long long unwrap(W w) {
  const auto u = w + 0u;
  // IR-DAG: code.cpp:[[@LINE-1]]:18: warning: Killed: Replaced w with ((void)(w), 42) [cxx_replace_scalar_call_nz]
  // IR-DAG: code.cpp:[[@LINE-2]]:18: warning: Killed: Replaced w with ((void)(w), 0) [cxx_replace_scalar_call_zr]
  return u + sizeof(u);
}

// The narrow instantiation comes first
long long unwrap_narrow(int n) {
  return unwrap(narrow{ n });
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced unwrap(narrow{ n }) with ((void)(narrow{ n }), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced unwrap(narrow{ n }) with ((void)(narrow{ n }), 0) [cxx_replace_scalar_call_zr]
}

long long unwrap_wide(int n) {
  return unwrap(wide{ n * 1000 });
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced unwrap(wide{ n * 1000 }) with ((void)(wide{ n * 1000 }), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced unwrap(wide{ n * 1000 }) with ((void)(wide{ n * 1000 }), 0) [cxx_replace_scalar_call_zr]
}
