int main() {
  try {
    throw 22;
    return 50;
  }
  catch (int x) {
    return 0;
  }

  return 42;
}

