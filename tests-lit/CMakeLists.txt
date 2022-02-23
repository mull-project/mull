set(LIT_EXEC lit)

find_package(Python3 COMPONENTS Interpreter)

if(APPLE)
  execute_process(
    COMMAND xcrun -show-sdk-path
    OUTPUT_VARIABLE SDKROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set (sysroot "--sysroot ${SDKROOT}")
else()
  set (sysroot)
endif()

set(TEST_CXX_FLAGS
  "-stdlib=libc++ -isystem ${CLANG_INSTALL_PREFIX}/include/c++/v1 -isystem ${CLANG_INSTALL_PREFIX}/lib/clang/${LLVM_VERSION}/include"
)

set(LIT_COMMAND
  PATH_TO_LLVM=${PATH_TO_LLVM}
  LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}
  CURRENT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  mull_runner=$<TARGET_FILE:mull-runner-${LLVM_VERSION_MAJOR}>
  mull_frontend_cxx=$<TARGET_FILE:mull-cxx-frontend-${LLVM_VERSION_MAJOR}>
  mull_ir_frontend=$<TARGET_FILE:mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR}>
  clang_cc=${LIT_CLANG}
  clang_cxx=${LIT_CLANGXX}
  sysroot=${sysroot}
  llvm_profdata=${LIT_LLVM_PROFDATA}
  python3=${Python3_EXECUTABLE}
  filecheck=filecheck
  TEST_CXX_FLAGS="${TEST_CXX_FLAGS}"
  ${LIT_EXEC}
  -vv
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
  )

add_custom_target(tests-lit
  COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && make clean
  COMMAND ${LIT_COMMAND}
  DEPENDS mull-runner-${LLVM_VERSION_MAJOR}
)
