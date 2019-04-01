set(google_test_include_dirs
  .
  include)

file(GLOB_RECURSE google_test_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(google_test_sources src/gtest-all.cc src/gtest_main.cc)

add_library(google-test STATIC EXCLUDE_FROM_ALL ${google_test_sources} ${google_test_headers})

set_target_properties(google-test
  PROPERTIES COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
target_include_directories(google-test PUBLIC
  ${google_test_include_dirs}
)

