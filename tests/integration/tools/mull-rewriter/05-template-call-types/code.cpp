// clang-format off
template <typename T> T id(T x) {
  return x;
}

template <typename T> struct wide_of {
  using type = unsigned long long;
};
template <> struct wide_of<int> {
  using type = unsigned;
};

// The call has a different type in each instantiation, 'auto' must keep it (#1231)
template <typename T> unsigned width() {
  using W = typename wide_of<T>::type;
  const auto w = id(static_cast<W>(1));
  // CHECK-DAG: code.cpp:[[@LINE-1]]:18: warning: Survived: Replaced id(static_cast<W>(1)) with ((void)(static_cast<W>(1)), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:18: warning: Survived: Replaced id(static_cast<W>(1)) with ((void)(static_cast<W>(1)), 0) [cxx_replace_scalar_call_zr]
  return sizeof(w);
}

// The wide instantiation comes first
unsigned wide_width() {
  return width<long>();
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced width<long>() with 42 [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced width<long>() with 0 [cxx_replace_scalar_call_zr]
}

unsigned narrow_width() {
  return width<int>();
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced width<int>() with 42 [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced width<int>() with 0 [cxx_replace_scalar_call_zr]
}
