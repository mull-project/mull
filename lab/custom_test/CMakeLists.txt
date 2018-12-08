set (SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/distance.c
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/test.c
)

foreach(source ${SOURCES})
  compile_fixture(
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm
  )
endforeach()

