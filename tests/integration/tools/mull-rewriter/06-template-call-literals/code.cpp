// clang-format off
template <typename T> T id(T x) {
  return x;
}

// The literal of a replaced call comes from the type of one instantiation: 'true' is 1, not 42
template <typename T> int get(T x) {
  return id(x);
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced id(x) with ((void)(x), {{42|true}}) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced id(x) with ((void)(x), {{0|false}}) [cxx_replace_scalar_call_zr]
}

// The bool instantiation comes first
int flag() {
  return get(true);
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced get(true) with ((void)(true), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced get(true) with ((void)(true), 0) [cxx_replace_scalar_call_zr]
}

int number() {
  return get(5);
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced get(5) with ((void)(5), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Survived: Replaced get(5) with ((void)(5), 0) [cxx_replace_scalar_call_zr]
}
