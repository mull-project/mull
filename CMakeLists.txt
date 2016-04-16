set(MUTANG_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MUTANG_INCLUDE_DIR  ${MUTANG_SOURCE_DIR}/include)
set(MUTANG_TOOLS_DIR    ${LLVM_TOOLS_BINARY_DIR})
set(MUTANG_BINARY_DIR   ${LLVM_BINARY_DIR})

include_directories(${MUTANG_INCLUDE_DIR})

add_subdirectory(tools)
add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(unittests)
