set (GTEST_DIR ${CMAKE_SOURCE_DIR}/googletest)

set (FLAGS
  -g
  -fno-exceptions
  -fno-rtti
  -fPIC
  -std=c++11
  -I${GTEST_DIR}/include
  -I${GTEST_DIR}
  -DGTEST_NO_LLVM_RAW_OSTREAM=1
  -c -emit-llvm
)

set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/Test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Testee.cpp
)

foreach(source ${SOURCES})
  compile_fixture(
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS ${FLAGS}
  )
endforeach()

