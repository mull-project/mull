add_subdirectory(Mutators/CXX)
add_subdirectory(Diagnostics)
add_subdirectory(Config)

set(mull_sources
  AST/ASTConstants.cpp
  AST/ASTScalarMutationMatcher.cpp
  AST/MullClangCompatibility.cpp

  Driver.cpp
  BitcodeMetadataReader.cpp
  MutationsFinder.cpp
  Mutant.cpp

  Mutators/CXX/LogicalAndToOr.cpp
  Mutators/CXX/LogicalOrToAnd.cpp
  Mutators/MutatorKind.cpp
  Mutators/MutatorsFactory.cpp
  Mutators/NegateConditionMutator.cpp
  Mutators/ScalarValueMutator.cpp

  Runner.cpp

  Bitcode.cpp
  MutationPoint.cpp
  FunctionUnderTest.cpp

  Metrics/MetricsMeasure.cpp

  JunkDetection/CXX/CXXJunkDetector.cpp

  Reporters/SourceCodeReader.cpp
  Reporters/SourceManager.cpp
  Reporters/SQLiteReporter.cpp
  Reporters/PatchesReporter.cpp
  Reporters/GithubAnnotationsReporter.cpp

  SourceLocation.cpp

  Parallelization/Progress.cpp
  Parallelization/TaskExecutor.cpp
  Parallelization/Tasks/SearchMutationPointsTask.cpp
  Parallelization/Tasks/DryRunMutantExecutionTask.cpp
  Parallelization/Tasks/MutantExecutionTask.cpp
  Parallelization/Tasks/MutantPreparationTasks.cpp
  Parallelization/Tasks/MutationFilterTask.cpp
  Parallelization/Tasks/ApplyMutationTask.cpp
  Parallelization/Tasks/FunctionFilterTask.cpp
  Parallelization/Tasks/InstructionSelectionTask.cpp

  Path.cpp

  Program/Program.cpp
  JunkDetection/CXX/Visitors/InstructionRangeVisitor.cpp
  JunkDetection/CXX/Visitors/NegateConditionVisitor.cpp
  JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.cpp
  JunkDetection/CXX/Visitors/ReplaceCallVisitor.cpp
  JunkDetection/CXX/Visitors/BinaryVisitor.cpp
  JunkDetection/CXX/Visitors/UnaryVisitor.cpp
  JunkDetection/CXX/ASTStorage.cpp
  JunkDetection/CXX/CompilationDatabase.cpp

  Reporters/IDEReporter.cpp
  Reporters/MutationTestingElementsReporter.cpp
  Reporters/SourceManager.cpp

  Filters/JunkMutationFilter.cpp
  Filters/NoDebugInfoFilter.cpp
  Filters/FilePathFilter.cpp
  Filters/GitDiffReader.cpp
  Filters/GitDiffFilter.cpp
  Filters/BlockAddressFunctionFilter.cpp
  Filters/Filters.cpp
  Filters/CoverageFilter.cpp
  Filters/VariadicFunctionFilter.cpp

  JunkDetection/CXX/Visitors/ScalarValueVisitor.cpp
  MutantRunner.cpp
)

set(MULL_INCLUDE_DIR ${MULL_SOURCE_DIR}/include/mull)

set(mull_header_dirs
  ${MULL_INCLUDE_DIR}
  ${MULL_INCLUDE_DIR}/Config
  ${MULL_INCLUDE_DIR}/JunkDetection
  ${MULL_INCLUDE_DIR}/JunkDetection/CXX/
  ${MULL_INCLUDE_DIR}/JunkDetection/CXX/Visitors
  ${MULL_INCLUDE_DIR}/Metrics
  ${MULL_INCLUDE_DIR}/Mutators
  ${MULL_INCLUDE_DIR}/MutationFilters
  ${MULL_INCLUDE_DIR}/Parallelization
  ${MULL_INCLUDE_DIR}/Parallelization/Tasks
  ${MULL_INCLUDE_DIR}/Program
  ${MULL_INCLUDE_DIR}/Reporters
  ${MULL_INCLUDE_DIR}/Toolchain
  )

# Globbing is not recommended by CMake so maybe consider listing headers
# explicitly in include/CMakeLists.txt file. However LLVM goes find(GLOB) under
# the hood so probably we are also good for now.
# "We do not recommend using GLOB to collect a list of source files from your
# source tree. If no CMakeLists.txt file changes when a source is added or
# removed then the generated build system cannot know when to ask
# CMake to regenerate. https://cmake.org/cmake/help/v3.0/command/file.html
file(GLOB_RECURSE mull_header_files ${MULL_INCLUDE_DIR}/*.h)

add_library(mull
  ${mull_sources}
  ${mull_header_files}
  )

set_source_files_properties(${mull_header_dirs} PROPERTIES HEADER_FILE_ONLY ON)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

target_link_libraries(mull
  LLVM
  SQLite::SQLite3
  clangTooling
  Threads::Threads
  irm
  mull-cxx-mutators
  mull-configuration
  Diagnostics
  reproc++
  )
target_include_directories(mull PUBLIC
  ${MULL_INCLUDE_DIRS}
)
target_include_directories(mull SYSTEM PRIVATE
  ${THIRD_PARTY_INCLUDE_DIRS}
)

set_target_properties(mull PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
