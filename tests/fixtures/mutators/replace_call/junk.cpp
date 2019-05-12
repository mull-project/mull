#define call_wrap(x) x()

extern "C" int external_function(int a, int b, int c);
extern "C" float external_function_f(float a, float b, float c);

int sum(int x, int b) {
  return x + b;
}

int bar();

int foo() {
  int r = sum(5, 8);
  int s = bar();
  int q = call_wrap(bar);
  int a = external_function(r, s, q);
  int b = external_function_f(r, s, q);

  return r + s + q + a + b;
}

class FooBar {
public:
  FooBar(int x) : i(x) {}

  int getValue() { return i; }
  int increment() { return i++; }
  int decrement() { return i--; }
private:
  int i;
};

struct Buzz {
  int x;
  int y;
  int z;
  int w;
};

int bar() {
  FooBar fbr(42);
  external_function(fbr.increment(), fbr.increment(), fbr.increment());

  external_function(fbr.getValue(), 0, 0);

  return fbr.decrement();
}

void buzz() {
  Buzz b;
  Buzz a = b;
  Buzz c;
  c = a;
}
