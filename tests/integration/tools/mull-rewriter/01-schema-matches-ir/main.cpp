extern "C" int printf(const char *, ...);
void consume(int) {}
int answer() {
  return 42;
}
bool yes() {
  return true;
}

int arithmetic(int a, int b) {
  int x = a + b;
  x = x - a;
  x = x * b;
  x = x / (b + 1);
  x = x % (a + 1);
  x += a;
  x -= b;
  x *= a;
  x /= (b + 1);
  x %= (a + 1);
  return -x;
}

int bitwise(int a, int b) {
  int x = (a & b) | (a ^ b);
  x = x << 1;
  x = x >> 1;
  x &= a;
  x |= b;
  x ^= a;
  x <<= 1;
  x >>= 1;
  return ~x;
}

int comparison(int a, int b) {
  int count = 0;
  if (a == b)
    count++;
  if (a != b)
    count++;
  if (a < b)
    count++;
  if (a <= b)
    count++;
  if (a > b)
    count++;
  if (a >= b)
    count++;
  return count;
}

int increments(int a) {
  int x = a;
  x++;
  x--;
  ++x;
  --x;
  return x;
}

bool logical(bool a, bool b) {
  if (!a)
    return b;
  return a && b || !b;
}

int negation(int a) {
  if (!a)
    return 1;
  return 0;
}

int calls(int a) {
  consume(a);
  int x = answer();
  bool flag = yes();
  return flag ? x : a;
}

int constants(int a) {
  int x = 5;
  double d = 2.5;
  bool flag = true;
  x = 7;
  d = 1.5;
  flag = false;
  x = a;
  return x + (int)d + flag;
}

int main() {
  for (int a = -3; a <= 3; a++)
    for (int b = -3; b <= 3; b++)
      printf("%d %d: %d %d %d %d %d %d %d %d\n",
             a,
             b,
             arithmetic(a, b + 4),
             bitwise(a, b),
             comparison(a, b),
             increments(a),
             logical(a > 0, b > 0),
             negation(a),
             calls(a),
             constants(a));
  return 0;
}
