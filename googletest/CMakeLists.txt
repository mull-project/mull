message(STATUS "googletest> Creating google-test target...")

set(google_test_include_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

include_directories(${google_test_include_dirs})

file(GLOB_RECURSE google_test_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(google_test_sources src/gtest-all.cc src/gtest_main.cc)

add_library(google-test SHARED ${google_test_sources} ${google_test_headers})
