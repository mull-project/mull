/// Taken from Wikipedia
/// https://en.wikipedia.org/wiki/Levenshtein_distance

int min(int a, int b) {
  if (a < b) {
    return a;
  }
  return b;
}

int minimum(int a, int b, int c) {
  int ab = min(a, b);
  return min(ab, c);
}

int LevenshteinDistance(const char *s, int len_s, const char *t, int len_t)
{
  int cost;

  /* base case: empty strings */
  if (len_s == 0) return len_t;
  if (len_t == 0) return len_s;

  /* test if last characters of the strings match */
  if (s[len_s-1] == t[len_t-1])
      cost = 0;
  else
      cost = 1;

  /* return minimum of delete char from s, delete char from t, and delete char from both */
  return minimum(LevenshteinDistance(s, len_s - 1, t, len_t    ) + 1,
                 LevenshteinDistance(s, len_s    , t, len_t - 1) + 1,
                 LevenshteinDistance(s, len_s - 1, t, len_t - 1) + cost);
}
