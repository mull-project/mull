// clang-format off
// Instantiated for int and unsigned: one mutant, each instantiation shifts with its own signedness
template <typename T> T shl1(T x) {
  return x << 1;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:12: warning: Killed: Replaced << with >> [cxx_lshift_to_rshift]
}

// Only unsigned: '>>' is logical, the mutant leaves the top bit clear
template <typename T> T top(T x) {
  return (x << 1) >> 31;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:13: warning: Survived: Replaced << with >> [cxx_lshift_to_rshift]
}

// Instantiated for pointers: 'p + q' is not a valid program
template <typename P> long distance(P p, P q) {
  return p - q;
  // No mutant: p and q are pointers
}

// Overloads above a template keep its operators CXXOperatorCallExprs in the pattern (#1226)
struct S {};
int operator<<(S, int);
bool operator<(S, S);
S &operator+=(S &, S);
S operator-(S);
S &operator++(S &);

template <typename T> T mix(T a, T b) {
  T r = a << 1;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:11: warning: Killed: Replaced << with >> [cxx_lshift_to_rshift]
  r += a < b;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced += with -= [cxx_add_assign_to_sub_assign]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced < with >= [cxx_lt_to_ge]
  r = -r;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:7: warning: Killed: Replaced - with  [cxx_minus_to_noop]
  ++r;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced ++ with -- [cxx_pre_inc_to_pre_dec]
  return r;
}

int signed_shift(int n) {
  return shl1(-4 * n);
}

unsigned unsigned_shift(unsigned n) {
  return shl1(0x80000000u * n);
}

unsigned top_bit(unsigned n) {
  return top(0x80000000u * n);
}

long length(int n) {
  const char *text = "hello";
  return distance(text + n, text);
}

int mixed(int n) {
  return mix(n, 2);
}
