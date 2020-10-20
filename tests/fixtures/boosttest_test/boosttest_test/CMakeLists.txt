set (FLAGS
  -g
  -fPIC
  -std=c++17
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

