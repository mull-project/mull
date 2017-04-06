message(STATUS "GoogleTest...")

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(google_test_sources src/gtest-all.cc src/gtest_main.cc)
set(google_test_headers "")

add_library(google-test
  SHARED ${google_test_sources} ${google_test_headers})
