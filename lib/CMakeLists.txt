set(mull_sources
  ConfigParser.cpp
  Config.cpp
  Context.cpp
  Driver.cpp
  ForkProcessSandbox.cpp
  Logger.cpp
  Mangler.cpp
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
)

set(mull_header_dirs
  ${MULL_INCLUDE_DIR}
  ${MULL_INCLUDE_DIR}/Mutators
  ${MULL_INCLUDE_DIR}/Context
  ${MULL_INCLUDE_DIR}/GoogleTest
  ${MULL_INCLUDE_DIR}/SimpleTest
  ${MULL_INCLUDE_DIR}/Toolchain
  ${MULL_INCLUDE_DIR}/Instrumentation
)

set(mull_additional_libraries
  ${MULL_DEPENDENCY_LLVM_LIBRARIES}

  ${MULL_DEPENDENCY_LIBZ}
  ${MULL_DEPENDENCY_NCURSES}

  ${MULL_DEPENDENCY_SQLITE}
  ${MULL_DEPENDENCY_LIBCLANG}
  ${MULL_DEPENDENCY_LIBCLANG_TOOLING}
  ${MULL_DEPENDENCY_LIBCLANG_BASIC}
)

set(mull_link_flags "") # to be filled later
set(mull_rpath "")      # to be filled later

if(MULL_SUPPORT_RUST)
  message(STATUS "Mull> MULL_SUPPORT_RUST detected.")

  set(mull_sources
    ${mull_sources}
    Rust/RustTest.cpp
    Rust/RustTestFinder.cpp
    Rust/RustTestRunner.cpp
    Rust/RustSymbolDemangling.cpp)

  set(mull_header_dirs
    ${mull_header_dirs}
    ${MULL_INCLUDE_DIR}/Rust)

  IF(NOT EXISTS ${MULL_RUST_LIB_PATH})
    message(FATAL_ERROR
      "MULL_RUST_LIB_PATH variable should be set to a valid \
      existing path in .mull.rust.cmake")
  endif()

  foreach(library ${MULL_DEPENDENCY_RUST_ADDITIONAL_LIBRARIES})
    IF(NOT EXISTS "${library}")
      message(FATAL_ERROR "Mull.Rust> path declared in \
        MULL_DEPENDENCY_RUST_ADDITIONAL_LIBRARIES does not exist: \
        ${library}")
    endif()
  endforeach()

  set(mull_additional_libraries
    ${mull_additional_libraries}
    ${MULL_DEPENDENCY_RUST_ADDITIONAL_LIBRARIES})

  set(mull_rpath ${mull_rpath} ${MULL_RUST_LIB_PATH})
  set(mull_link_flags ${mull_link_flags} -L${MULL_RUST_LIB_PATH})
endif()

# Globbing is not recommended by CMake so maybe consider listing headers
# explicitly in include/CMakeLists.txt file. However LLVM goes find(GLOB) under
# the hood so probably we are also good for now.
# "We do not recommend using GLOB to collect a list of source files from your
# source tree. If no CMakeLists.txt file changes when a source is added or
# removed then the generated build system cannot know when to ask
# CMake to regenerate. https://cmake.org/cmake/help/v3.0/command/file.html
file(GLOB_RECURSE mull_header_files ${MULL_INCLUDE_DIR}/*.h)

add_library(mull SHARED ${mull_sources} ${mull_header_files})

set_source_files_properties(${mull_header_dirs} PROPERTIES HEADER_FILE_ONLY ON)
include_directories(${mull_header_dirs})

target_link_libraries(mull ${mull_additional_libraries})

## Once we have mull target added, we can start querying and patching it.
# rpath
get_target_property(default_rpath mull INSTALL_RPATH)
set(mull_rpath ${default_rpath} ${mull_rpath})
set_target_properties(mull PROPERTIES INSTALL_RPATH "${mull_rpath}")

# compile flags
get_target_property(default_compile_flags mull COMPILE_FLAGS)
if(NOT default_compile_flags)
  set(default_compile_flags "")
endif()
set(mull_compileflags ${default_compile_flags} ${LLVM_CXX_FLAGS})
set_target_properties(mull
  PROPERTIES COMPILE_FLAGS
  "${mull_compileflags}"
)

# Link flags
get_target_property(default_link_flags mull LINK_FLAGS)
if(NOT ${default_link_flags})
set(default_link_flags "")
endif()
set(mull_link_flags
  "${default_link_flags} ${LLVM_LINK_FLAGS}"
)
set_target_properties(mull PROPERTIES LINK_FLAGS "${mull_link_flags}")

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
)
