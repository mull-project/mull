// clang-format off
#define LIMIT 100

struct vec {
  int size = 0;
  void resize(int n) {
    size = n;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  }
  int get() const {
    return size;
  }
};

struct big {
  vec digits;
  int exp = 5;
  template <typename T> void assign(T n) {
    digits.resize(n);
    exp = 0;
    // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced = with = 1; [cxx_assign_const]
  }
};

enum class color { red, green };
color pick(int n) {
  return n ? color::green : color::red;
}

struct wrapped {
  int value;
  explicit operator int() const {
    return value;
  }
};

struct number {
  int value;
  number operator+(number other) const {
    return { value * other.value };
    // CHECK: code.cpp:[[@LINE-1]]:20: warning: Killed: Replaced * with / [cxx_mul_to_div]
  }
};

template <typename T> T twice(T value) {
  // No source equivalent: twice<number> has no '-'
  return value + value;
  // IR: code.cpp:[[@LINE-1]]:16: warning: Killed: Replaced + with - [cxx_add_to_sub]
}

long distance(const char *begin, const char *end) {
  return end - begin;
}

int resized(int n) {
  vec v;
  v.resize(n + 1);
  // CHECK: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced v.resize(n + 1) with ((void)(v), (void)(n + 1)) [cxx_remove_void_call]
  // CHECK: code.cpp:[[@LINE-2]]:14: warning: Killed: Replaced + with - [cxx_add_to_sub]
  return v.get();
  // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced v.get() with ((void)(v), 42) [cxx_replace_scalar_call_nz]
  // CHECK: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced v.get() with ((void)(v), 0) [cxx_replace_scalar_call_zr]
}

int assigned_digits(int n) {
  big b;
  b.assign(unsigned(n));
  // CHECK: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced b.assign(unsigned(n)) with ((void)(b), (void)(unsigned(n))) [cxx_remove_void_call]
  return b.digits.size;
}

int assigned_exp(int n) {
  big b;
  b.assign(long(n));
  // CHECK: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced b.assign(long(n)) with ((void)(b), (void)(long(n))) [cxx_remove_void_call]
  return b.exp;
}

int limit() {
  int limit = LIMIT;
  // CHECK: code.cpp:[[@LINE-1]]:15: warning: Killed: Replaced LIMIT with 101 [cxx_init_const]
  return limit;
}

char letter() {
  auto letter = 'a';
  // CHECK: code.cpp:[[@LINE-1]]:17: warning: Killed: Replaced 'a' with 96 [cxx_init_const]
  return letter;
}

color picked(int n) {
  auto c = pick(n);
  return c;
}

int unwrapped(int n) {
  wrapped w{ n + 6 };
  // CHECK: code.cpp:[[@LINE-1]]:16: warning: Killed: Replaced + with - [cxx_add_to_sub]
  int unwrapped = static_cast<int>(w);
  // CHECK: code.cpp:[[@LINE-1]]:19: warning: Killed: Replaced static_cast<int>(w) with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:36: warning: Killed: Replaced w with ((void)(w), 42) [cxx_replace_scalar_call_nz]
  // CHECK: code.cpp:[[@LINE-3]]:36: warning: Killed: Replaced w with ((void)(w), 0) [cxx_replace_scalar_call_zr]
  return unwrapped;
}

int sum(int n) {
  int sum = twice(n) + twice(number{ n + 1 }).value;
  // CHECK: code.cpp:[[@LINE-1]]:13: warning: Killed: Replaced twice(n) + twice(number{ n + 1 }).value with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:13: warning: Killed: Replaced twice(n) with ((void)(n), 42) [cxx_replace_scalar_call_nz]
  // CHECK: code.cpp:[[@LINE-3]]:13: warning: Killed: Replaced twice(n) with ((void)(n), 0) [cxx_replace_scalar_call_zr]
  // CHECK: code.cpp:[[@LINE-4]]:22: warning: Killed: Replaced + with - [cxx_add_to_sub]
  // CHECK: code.cpp:[[@LINE-5]]:40: warning: Killed: Replaced + with - [cxx_add_to_sub]
  return sum;
}

long length(int n) {
  const char *text = "hello";
  return distance(text, text + n);
  // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced distance(text, text + n) with ((void)(text), (void)(text + n), 42) [cxx_replace_scalar_call_nz]
  // CHECK: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced distance(text, text + n) with ((void)(text), (void)(text + n), 0) [cxx_replace_scalar_call_zr]
}
