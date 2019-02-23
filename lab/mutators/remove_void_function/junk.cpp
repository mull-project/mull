#define call_void(x) x()

extern "C" void *memcpy(void *dst, const void *src, int n);
extern "C" void external_function(int x = 0);

void sum(int x, int b, int *res) {
  *res = x + b;
}

void bar();

int foo() {
  int r = 0;
  sum(5, 8, &r);

  bar();
  call_void(bar);
  external_function();

  return r;
}

class FooBar {
public:
  FooBar(int x) : i(x) {}

  int getValue() { return i; }
  void increment() { i++; }
  void decrement() { i--; }
private:
  int i;
};

struct Buzz {
  int x;
  int y;
  int z;
  int w;
};

void bar() {
  FooBar fbr(42);
  fbr.increment();
  fbr.decrement();

  external_function(fbr.getValue());
}

void buzz() {
  /// Expected to contain bunch of mem copy intrinsics
  Buzz b;
  Buzz a = b;
  Buzz c;
  c = a;
}

