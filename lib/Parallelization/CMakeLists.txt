add_library(mull-parallelization
  Progress.cpp
  TaskExecutor.cpp
  Tasks/SearchMutationPointsTask.cpp
  Tasks/DryRunMutantExecutionTask.cpp
  Tasks/MutantExecutionTask.cpp
  Tasks/MutantPreparationTasks.cpp
  Tasks/MutationFilterTask.cpp
  Tasks/ApplyMutationTask.cpp
  Tasks/FunctionFilterTask.cpp
  Tasks/InstructionSelectionTask.cpp
)
find_package(Threads REQUIRED)
target_link_libraries(mull-parallelization PRIVATE
  Threads::Threads
  mull-core
  mull-configuration
  mull-diagnostics
)
