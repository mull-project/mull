add_executable(mull-runner-${LLVM_VERSION_MAJOR}
  mull-runner.cpp
  MutantExtractor.cpp
  DynamicLibraries.cpp
  ../CLIOptions/CLIOptions.cpp
  MergeInstProfile.cpp
)
target_include_directories(mull-runner-${LLVM_VERSION_MAJOR} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../CLIOptions)
target_link_libraries(mull-runner-${LLVM_VERSION_MAJOR}
  LLVM
  mull-core
  mull-diagnostics
  mull-configuration
  mull-filters
  mull-cxx-mutators
  mull-reporters
  mull-source-manager
  mull-parallelization
  mull-runner
  mull-misc
  mull-version
  # FIXME: this should not be here
  mull-mutation
)
install(TARGETS mull-runner-${LLVM_VERSION_MAJOR}
  RUNTIME DESTINATION bin
)
