set(MULL_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MULL_INCLUDE_DIR  ${MULL_SOURCE_DIR}/include)
set(MULL_TOOLS_DIR    ${LLVM_TOOLS_BINARY_DIR})
set(MULL_BINARY_DIR   ${LLVM_BINARY_DIR})

include_directories(${MULL_INCLUDE_DIR})

add_subdirectory(tools)
add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(unittests)

