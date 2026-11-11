// clang-format off
// Not mutated, defined in tests.cpp: every call of advance() moves the position
extern "C" int position;
extern "C" int advance();
extern "C" void ignore(int value);
extern "C" bool present(const int *value);

struct pair {
  int first;
  int second;
};
void take(pair) {}

struct counter {
  int value;
  int get() const {
    return value;
  }
};

int *fill(int *value) {
  *value = 7;
  return value;
}

counter next() {
  return counter{ advance() };
  // CHECK-DAG: code.cpp:[[@LINE-1]]:19: warning: Killed: Replaced advance() with 42 [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:19: warning: Killed: Replaced advance() with 0 [cxx_replace_scalar_call_zr]
}

// The argument of a removed call is still evaluated
int ignored() {
  int start = position;
  ignore(advance());
  // CHECK-DAG: code.cpp:[[@LINE-1]]:3: warning: Survived: Replaced ignore(advance()) with (void)(advance()) [cxx_remove_void_call]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced advance() with 42 [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-3]]:10: warning: Killed: Replaced advance() with 0 [cxx_replace_scalar_call_zr]
  return position - start;
}

// The argument of a replaced call is still evaluated: present() is true anyway
int filled() {
  int value = 0;
  return present(fill(&value)) ? value : -1;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Survived: Replaced present(fill(&value)) with ((void)(fill(&value)), true) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced present(fill(&value)) with ((void)(fill(&value)), false) [cxx_replace_scalar_call_zr]
}

// The object of a replaced member call is still evaluated
int counted() {
  int start = position;
  return next().get() > 0 ? position - start : -1;
  // CHECK-DAG: code.cpp:[[@LINE-1]]:10: warning: Survived: Replaced next().get() with ((void)(next()), 42) [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced next().get() with ((void)(next()), 0) [cxx_replace_scalar_call_zr]
}

// Every argument is kept, like in the IR
int unused(int n) {
  ignore(n);
  // CHECK-DAG: code.cpp:[[@LINE-1]]:3: warning: Survived: Replaced ignore(n) with (void)(n) [cxx_remove_void_call]
  return n;
}

// A braced initializer is kept as its elements, '(void)({ advance(), 1 })' does not compile
int taken() {
  int start = position;
  take({ advance(), 1 });
  // CHECK-DAG: code.cpp:[[@LINE-1]]:3: warning: Survived: Replaced take({ advance(), 1 }) with ((void)(advance()), (void)(1)) [cxx_remove_void_call]
  // CHECK-DAG: code.cpp:[[@LINE-2]]:10: warning: Killed: Replaced advance() with 42 [cxx_replace_scalar_call_nz]
  // CHECK-DAG: code.cpp:[[@LINE-3]]:10: warning: Killed: Replaced advance() with 0 [cxx_replace_scalar_call_zr]
  return position - start;
}
