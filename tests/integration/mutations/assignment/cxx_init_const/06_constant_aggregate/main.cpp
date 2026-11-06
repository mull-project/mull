#include <time.h>
constexpr tm make_tm(int year) {
  auto t = tm();
  t.tm_year = year - 1900;
  return t;
}
int main() {
  auto t = make_tm(1975);
  int year = 75;
  return t.tm_year != year;
}
