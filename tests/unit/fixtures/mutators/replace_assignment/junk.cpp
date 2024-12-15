struct point {
  int x;
  int y;
};

extern int externIntegerCall();
extern double externDoubleCall();
extern float externFloatCall();

extern char *externCharPtrCall();

extern int externInteger;
extern double externDouble;
extern float externFloat;

void test1() {
  auto t1 = externIntegerCall();
  auto t2 = externDoubleCall();
  auto t3 = externFloatCall();
  auto t4 = externCharPtrCall();
}

void test2() {
  int t1;
  t1 = externInteger;
  double t2;
  t2 = externDoubleCall();
  float t3;
  t3 = externFloat;
  char *t4;
  t4 = externCharPtrCall();
}

void test3() {
  point p;
  p.x = externInteger;
  p.y = externIntegerCall();
}

void test4() {
  auto t1(externInteger);
  auto t2(externFloatCall());
  auto t3(externDouble);
}