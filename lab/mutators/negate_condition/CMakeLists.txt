set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/tester.c
  ${CMAKE_CURRENT_LIST_DIR}/testee.c
)

foreach(source ${SOURCES})
  compile_fixture(
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -g -c -emit-llvm
  )
endforeach()

