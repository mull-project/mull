set (CURRENT_CC ${PRECOMPILED_LLVM_DIR}/bin/clang)
set (CURRENT_CXX ${PRECOMPILED_LLVM_DIR}/bin/clang++)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.common.in
  ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.common
)
