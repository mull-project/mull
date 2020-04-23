set(CLANG_EXEC ${PATH_TO_LLVM}/bin/clang++)
set(LIT_EXEC lit)
set(FILECHECK_EXEC filecheck)

if(APPLE)
  execute_process(
    COMMAND xcrun -show-sdk-path
    OUTPUT_VARIABLE SDKROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set (SYSROOT "--sysroot ${SDKROOT}")
else()
  set (SYSROOT)
endif()

set(TEST_CXX_FLAGS
  "-stdlib=libc++ -isystem ${PATH_TO_LLVM}/include/c++/v1 -isystem ${PATH_TO_LLVM}/lib/clang/${LLVM_VERSION}/include"
)

if (SYSROOT)
  set(TEST_CXX_FLAGS "${TEST_CXX_FLAGS} ${SYSROOT}")
endif()

set(LIT_COMMAND
  PATH_TO_LLVM=${PATH_TO_LLVM}
  LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}
  CURRENT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  MULL_EXEC=$<TARGET_FILE:mull-cxx>
  CLANG_EXEC=${CLANG_EXEC}
  FILECHECK_EXEC=${FILECHECK_EXEC}
  TEST_CXX_FLAGS="${TEST_CXX_FLAGS}"
  ${LIT_EXEC}
  --threads 1 # we want to remove this when Mull can be parallelized
  -vv
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_custom_target(tests-lit
  COMMAND ${LIT_COMMAND}
  DEPENDS mull-cxx
)
