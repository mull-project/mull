add_library(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} SHARED mull-cxx-ir-frontend.cpp)
target_link_libraries(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR}
  PRIVATE
  LLVM
  clangTooling
  mull-core
  mull-diagnostics
  mull-configuration
  mull-filters
  mull-junk-detection
  mull-cxx-mutators
  mull-mutation
  mull-source-manager
  mull-ast
  mull-parallelization
  mull-runner
  mull-ir-frontend
  mull-misc
)
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES LIBRARY_OUTPUT_NAME mull-ir-frontend-${LLVM_VERSION_MAJOR})
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES PREFIX "")
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES SUFFIX "")
install(TARGETS mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR}
  RUNTIME DESTINATION lib
)
