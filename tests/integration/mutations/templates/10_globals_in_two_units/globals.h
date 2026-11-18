#pragma once

inline int base() {
  return 1;
}

template <typename T> struct Counter {
  static T start;
};

template <typename T> T Counter<T>::start = T(base()) + 1;

template <typename T> T scaled = T(base()) + 2;

inline int shared = base() + 3;
