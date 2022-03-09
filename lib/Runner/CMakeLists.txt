add_library(mull-runner
  MutantRunner.cpp
  Runner.cpp
)
target_link_libraries(mull-runner PRIVATE reproc++ mull-parallelization)
