extern "C" int printf(const char *, ...);
#define LIMIT 100

struct vec {
  int size = 0;
  void resize(int n) {
    size = n;
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
  }
};

template <typename T> T twice(T value) {
  return value + value;
}

long distance(const char *begin, const char *end) {
  return end - begin;
}

int main(int argc, char **) {
  big b;
  b.assign(3u);
  b.assign(4l);
  vec v;
  v.resize(argc + 1);
  int limit = LIMIT;
  auto letter = 'a';
  auto c = pick(argc);
  wrapped w{ argc + 6 };
  int unwrapped = static_cast<int>(w);
  int sum = twice(argc) + twice(number{ argc + 1 }).value;
  const char *text = "hello";
  printf("%d %d %d %d %c %d %d %d %ld\n",
         b.digits.size,
         b.exp,
         v.get(),
         limit,
         letter,
         int(c),
         unwrapped,
         sum,
         distance(text, text + 5));
  return 0;
}
