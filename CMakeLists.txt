cmake_minimum_required(VERSION 3.5.1)

project(Mull)
enable_language(C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/properties.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/fixtures.cmake)

if (NOT PRECOMPILED_LLVM_DIR AND NOT SOURCE_LLVM_DIR)
  message(FATAL_ERROR " 
 The cmake is supposed to be called with either of the following options:
 PRECOMPILED_LLVM_DIR: path to a precompiled version of LLVM
 SOURCE_LLVM_DIR: path to a source code of LLVM
 
 Examples:
 cmake -G \"${CMAKE_GENERATOR}\" -DPRECOMPILED_LLVM_DIR=/opt/llvm-3.9.0 ${CMAKE_SOURCE_DIR}
 cmake -G \"${CMAKE_GENERATOR}\" -DSOURCE_LLVM_DIR=${CMAKE_SOURCE_DIR}/llvm_source/3.9.0 ${CMAKE_SOURCE_DIR}
")
endif()

# This enables assertions for Release builds.
# https://stackoverflow.com/questions/22140520/how-to-enable-assert-in-cmake-release-mode
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Using system's SQLite version by default
# But also trying to find it using PkgConfig
# See https://github.com/mull-project/mull/issues/401 for details
set (SQLITE_DEPENDENCY sqlite3)
find_package(PkgConfig)
if (PkgConfig_FOUND)
  pkg_check_modules(SQLITE3 sqlite3 IMPORTED_TARGET)
  if (SQLITE3_FOUND)
    set (SQLITE_DEPENDENCY PkgConfig::SQLITE3)
  endif()
endif()

if (PRECOMPILED_LLVM_DIR)
  set (search_paths
    ${PRECOMPILED_LLVM_DIR}
    ${PRECOMPILED_LLVM_DIR}/lib/cmake
    ${PRECOMPILED_LLVM_DIR}/lib/cmake/llvm
    ${PRECOMPILED_LLVM_DIR}/lib/cmake/clang
    ${PRECOMPILED_LLVM_DIR}/share/clang/cmake/
    ${PRECOMPILED_LLVM_DIR}/share/llvm/cmake/
  )

  find_package(LLVM REQUIRED CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)
  find_package(Clang REQUIRED CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)

  if (TARGET clang)
    get_target_property(MULL_CC clang LOCATION)
  else()
    set(MULL_CC ${PRECOMPILED_LLVM_DIR}/bin/clang)
  endif()
endif()

if(SOURCE_LLVM_DIR)
  macro(get_llvm_version_component input component)
    string(REGEX MATCH "${component} ([0-9]+)" match ${input})
    if (NOT match)
      message(FATAL_ERROR "Cannot find LLVM version component '${component}'")
    endif()
    set (${component} ${CMAKE_MATCH_1})
  endmacro()

  file(READ ${SOURCE_LLVM_DIR}/CMakeLists.txt LLVM_CMAKELISTS)
  get_llvm_version_component("${LLVM_CMAKELISTS}" LLVM_VERSION_MAJOR)
  get_llvm_version_component("${LLVM_CMAKELISTS}" LLVM_VERSION_MINOR)
  get_llvm_version_component("${LLVM_CMAKELISTS}" LLVM_VERSION_PATCH)

  add_subdirectory(${SOURCE_LLVM_DIR} llllllxc)

  if (NOT TARGET clangTooling)
    message(FATAL_ERROR " 
 Cannot find clangTooling target. Did you forget to clone clang sources?
 Clean CMake cache and make sure they are available at:
 ${SOURCE_LLVM_DIR}/tools/clang")
  endif()

  # Normally, include paths provided by LLVMConfig.cmake
  # In this case we can 'steal' them from real targets
  get_target_property(llvm_support_includes LLVMSupport INCLUDE_DIRECTORIES)
  get_target_property(clang_tooling_includes clangTooling INCLUDE_DIRECTORIES)
  set(LLVM_INCLUDE_DIRS ${llvm_support_includes} ${clang_tooling_includes})
  list(REMOVE_DUPLICATES LLVM_INCLUDE_DIRS)

  get_target_property(clang_build_dir clang BUILD_DIR)
  message("${clang_build_dir}")
endif()

set (llvm_patch_version "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}")
set (llvm_minor_version "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.x")
set (llvm_major_version "${LLVM_VERSION_MAJOR}.x.x")

set (full_llvm_version ${llvm_patch_version})

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${llvm_patch_version})
  set (LLVM_COMPATIBILITY_DIR ${llvm_patch_version})

elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${llvm_minor_version})
  set (LLVM_COMPATIBILITY_DIR ${llvm_minor_version})

elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${llvm_major_version})
  set (LLVM_COMPATIBILITY_DIR ${llvm_major_version})

else()
  message(FATAL_ERROR "LLVM-${full_llvm_version} is not supported")
endif()

set(MULL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (MULL_CXX_FLAGS "-std=c++11 -fPIC -fvisibility-inlines-hidden -fno-exceptions")
if (NOT LLVM_ENABLE_RTTI)
  set(MULL_CXX_FLAGS "${MULL_CXX_FLAGS} -fno-rtti")
endif()

set (MULL_LINK_FLAGS "-rdynamic")
set (MULL_DEFINITIONS ${LLVM_DEFINITIONS})
set (MULL_INCLUDE_DIRS
  ${MULL_SOURCE_DIR}/include
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${LLVM_COMPATIBILITY_DIR}
  ${SQLITE3_INCLUDE_DIRS}
  ${SQLITE3_INCLUDEDIR}
)

# https://cmake.org/Wiki/CMake_RPATH_handling#Always_full_RPATH
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" "${LLVM_LIB_PATH}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(lab)
add_subdirectory(LLVMCompatibility/${LLVM_COMPATIBILITY_DIR})
add_subdirectory(lib)
add_subdirectory(googletest)
add_subdirectory(tools)
add_subdirectory(unittests)

