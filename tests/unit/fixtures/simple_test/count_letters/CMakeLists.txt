set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/test_count_letters.c
  ${CMAKE_CURRENT_LIST_DIR}/count_letters.c
)

foreach(source ${SOURCES})
  compile_fixture(
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -g -c -emit-llvm
  )
endforeach()

