set (CPPUTEST_DIR ${CMAKE_SOURCE_DIR}/vendor/cpputest)

set (FLAGS
  -g
  -fno-exceptions
  -fno-rtti
  -fPIC
  -std=c++11
  -I${CPPUTEST_DIR}/include
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

