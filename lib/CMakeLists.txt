set(mull_sources
  ConfigParser.cpp
  Config.cpp
  Context.cpp
  Driver.cpp
  ForkProcessSandbox.cpp
  Logger.cpp
  ModuleLoader.cpp
  Filter.cpp
  MutationsFinder.cpp

  Instrumentation/DynamicCallTree.cpp
  Instrumentation/Callbacks.cpp
  Instrumentation/Instrumentation.cpp

  Mutators/MathAddMutator.cpp
  Mutators/AndOrReplacementMutator.cpp
  Mutators/MutatorsFactory.cpp
  Mutators/NegateConditionMutator.cpp
  Mutators/RemoveVoidFunctionMutator.cpp
  Mutators/ReplaceAssignmentMutator.cpp
  Mutators/ReplaceCallMutator.cpp
  Mutators/MathDivMutator.cpp
  Mutators/MathMulMutator.cpp
  Mutators/MathSubMutator.cpp
  Mutators/ScalarValueMutator.cpp

  Mutators/ConditionalsBoundaryMutator.cpp

  Toolchain/Compiler.cpp
  Toolchain/ObjectCache.cpp
  Toolchain/Toolchain.cpp
  Toolchain/JITEngine.cpp
  Toolchain/Mangler.cpp
  Toolchain/Resolvers/InstrumentationResolver.cpp
  Toolchain/Resolvers/NativeResolver.cpp

  MullModule.cpp
  MutationPoint.cpp
  TestRunner.cpp
  Testee.cpp

  SimpleTest/SimpleTest_Test.cpp
  SimpleTest/SimpleTestFinder.cpp
  SimpleTest/SimpleTestRunner.cpp

  GoogleTest/GoogleTest_Test.cpp
  GoogleTest/GoogleTestFinder.cpp
  GoogleTest/GoogleTestRunner.cpp

  CustomTestFramework/CustomTest_Test.cpp
  CustomTestFramework/CustomTestRunner.cpp
  CustomTestFramework/CustomTestFinder.cpp

  IDEDiagnostics.cpp

  Metrics/Metrics.cpp

  JunkDetection/CXX/CXXJunkDetector.cpp

  Reporters/SQLiteReporter.cpp
  Reporters/TimeReporter.cpp
  SourceLocation.cpp

  Parallelization/Progress.cpp
  Parallelization/TaskExecutor.cpp
  Parallelization/Tasks/ModuleLoadingTask.cpp
  Parallelization/Tasks/SearchMutationPointsTask.cpp
  Parallelization/Tasks/LoadObjectFilesTask.cpp
  Parallelization/Tasks/DryRunMutantExecutionTask.cpp
  Parallelization/Tasks/InstrumentedCompilationTask.cpp
  Parallelization/Tasks/OriginalTestExecutionTask.cpp
  Parallelization/Tasks/JunkDetectionTask.cpp
  Parallelization/Tasks/MutantExecutionTask.cpp
  Parallelization/Tasks/OriginalCompilationTask.cpp
)

set (MULL_INCLUDE_DIR ${MULL_SOURCE_DIR}/include)

set(mull_header_dirs
  ${MULL_INCLUDE_DIR}
  ${MULL_INCLUDE_DIR}/Mutators
  ${MULL_INCLUDE_DIR}/Context
  ${MULL_INCLUDE_DIR}/GoogleTest
  ${MULL_INCLUDE_DIR}/SimpleTest
  ${MULL_INCLUDE_DIR}/Toolchain
  ${MULL_INCLUDE_DIR}/Instrumentation
  ${MULL_INCLUDE_DIR}/Parallelization
  ${MULL_INCLUDE_DIR}/Parallelization/Tasks
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
  $<TARGET_OBJECTS:LLVMCompatibility>
)

set_source_files_properties(${mull_header_dirs} PROPERTIES HEADER_FILE_ONLY ON)

if ("LLVM" IN_LIST LLVM_AVAILABLE_LIBS)
  set (MULL_LLVM_LIBRARIES
    LLVM
  )
else()
  set (MULL_LLVM_LIBRARIES
    LLVMAsmParser
    LLVMOrcJIT
    LLVMSupport
    LLVMOption
    LLVMX86CodeGen
    LLVMX86AsmParser
  )
endif()

find_package(Threads REQUIRED)

target_link_libraries(mull
  ${MULL_LLVM_LIBRARIES}
  ${SQLITE_DEPENDENCY}
  libclang
  clangTooling
  Threads::Threads
)
target_include_directories(mull PUBLIC ${MULL_INCLUDE_DIRS})

set_target_properties(mull PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
  LINK_FLAGS ${MULL_LINK_FLAGS}
)

# The following is needed to make CMake to generate a folder structure in Xcode
# that corresponds to a physical folder structure.
get_target_property(HERE_TARGET_SOURCES mull SOURCES)
foreach(source_file ${HERE_TARGET_SOURCES})
  if (source_file MATCHES "(.*)\.h$")
    file(RELATIVE_PATH relative_source_file ${MULL_INCLUDE_DIR} ${source_file})
    get_filename_component(relative_path ${relative_source_file} DIRECTORY)
    source_group("Header Files\\${relative_path}" FILES ${source_file})
  else()
    get_filename_component(relative_path ${source_file} DIRECTORY)
    source_group("Source Files\\${relative_path}" FILES ${source_file})
  endif()
endforeach()

INSTALL(TARGETS mull
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

