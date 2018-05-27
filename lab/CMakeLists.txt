set (CURRENT_CC ${LLVM_ROOT}/bin/clang)
set (CURRENT_CXX ${LLVM_ROOT}/bin/clang++)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.common.in
  ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.common
)
