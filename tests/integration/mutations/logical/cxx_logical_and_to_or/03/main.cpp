// Reduced from https://github.com/mull-project/mull/issues/1077
struct s {
  int a;
  int b;
  bool p() const {
    return a > b;
  }

  bool f(s const &other) {
    return (this->p() && other.p()) || (this->a == other.a && this->b == other.b);
  }
};

int main() {
  s s;
  return s.f(s);
}

// clang-format off

// RUN: %clang_cxx %sysroot %TEST_CXX_FLAGS -O0 %pass_mull_ir_frontend -fprofile-instr-generate -g %s -o %s.exe
