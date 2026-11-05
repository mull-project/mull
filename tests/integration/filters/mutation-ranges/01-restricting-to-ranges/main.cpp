int first(int, int);
int second(int, int);
int fourth(int, int);
int main() {
  return first(1, 2) == 3 && second(1, 2) == 3 && fourth(1, 2) == 3 ? 0 : 1;
}
