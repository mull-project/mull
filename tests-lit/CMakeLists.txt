set(CLANG_EXEC ${PATH_TO_LLVM}/bin/clang++)
set(LIT_EXEC lit)
set(FILECHECK_EXEC filecheck)

set(LIT_COMMAND
  LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}
  CURRENT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  MULL_EXEC=$<TARGET_FILE:mull-cxx>
  CLANG_EXEC=${CLANG_EXEC}
  FILECHECK_EXEC=${FILECHECK_EXEC}
  ${LIT_EXEC}
  --threads 1 # we want to remove this when Mull can be parallelized
  -vv
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_custom_target(tests-lit
  COMMAND ${LIT_COMMAND}
  DEPENDS mull-cxx
)
