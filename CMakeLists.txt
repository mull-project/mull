cmake_minimum_required(VERSION 3.7.0)

if (NOT MULL_VERSION)
set (MULL_VERSION 0.7.0)
endif()

project(Mull
  LANGUAGES C CXX
)

# -DRECORD_TIMING=ON
if (RECORD_TIMING)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "\"${CMAKE_COMMAND}\" -E time")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "\"${CMAKE_COMMAND}\" -E time")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_CUSTOM "\"${CMAKE_COMMAND}\" -E time")
endif()

set (PROJECT_VERSION ${MULL_VERSION})

set (PROJECT_DESCRIPTION "LLVM-based mutation testing")
set (PROJECT_HOMEPAGE_URL "https://github.com/mull-project/mull")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/properties.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/fixtures.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/functions.cmake)

if (PRECOMPILED_LLVM_DIR)
  message(WARNING "PRECOMPILED_LLVM_DIR is deprecated. Please, use PATH_TO_LLVM instead")
  set(PATH_TO_LLVM ${PRECOMPILED_LLVM_DIR})
endif()

if (SOURCE_LLVM_DIR)
  message(WARNING "SOURCE_LLVM_DIR is deprecated. Please, use PATH_TO_LLVM instead")
  set(PATH_TO_LLVM ${SOURCE_LLVM_DIR})
endif()

if (NOT PATH_TO_LLVM)
  message(FATAL_ERROR " 
  The cmake is supposed to be called with PATH_TO_LLVM pointing to
 a precompiled version of LLVM or to to the source code of LLVM
 Examples:
 cmake -G \"${CMAKE_GENERATOR}\" -DPATH_TO_LLVM=/opt/llvm-3.9.0 ${CMAKE_SOURCE_DIR}
 cmake -G \"${CMAKE_GENERATOR}\" -DPATH_TO_LLVM=/opt/llvm/source ${CMAKE_SOURCE_DIR}
")
endif()

if (NOT IS_ABSOLUTE ${PATH_TO_LLVM})
  # Convert relative path to absolute path
  get_filename_component(PATH_TO_LLVM
    "${PATH_TO_LLVM}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
endif()

set (BUILD_AGAINST_PRECOMPILED_LLVM TRUE)
if (EXISTS ${PATH_TO_LLVM}/CMakeLists.txt)
  set (BUILD_AGAINST_PRECOMPILED_LLVM FALSE)
endif()

# This enables assertions for Release builds.
# https://stackoverflow.com/questions/22140520/how-to-enable-assert-in-cmake-release-mode
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

option(MULL_BUILD_32_BITS "Enable 32 bits build" OFF)

if (${BUILD_AGAINST_PRECOMPILED_LLVM})
  set (search_paths
    ${PATH_TO_LLVM}
    ${PATH_TO_LLVM}/lib/cmake
    ${PATH_TO_LLVM}/lib/cmake/llvm
    ${PATH_TO_LLVM}/lib/cmake/clang
    ${PATH_TO_LLVM}/share/clang/cmake/
    ${PATH_TO_LLVM}/share/llvm/cmake/
  )

  find_package(LLVM REQUIRED CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)
  find_package(Clang REQUIRED CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)

  if (TARGET clang)
    get_target_property(MULL_CC clang LOCATION)
  else()
    set(MULL_CC ${PATH_TO_LLVM}/bin/clang)
  endif()
else()
  macro(get_llvm_version_component input component)
    string(REGEX MATCH "${component} ([0-9]+)" match ${input})
    if (NOT match)
      message(FATAL_ERROR "Cannot find LLVM version component '${component}'")
    endif()
    set (${component} ${CMAKE_MATCH_1})
  endmacro()

  file(READ ${PATH_TO_LLVM}/CMakeLists.txt LLVM_CMAKELISTS)
  get_llvm_version_component("${LLVM_CMAKELISTS}" LLVM_VERSION_MAJOR)
  get_llvm_version_component("${LLVM_CMAKELISTS}" LLVM_VERSION_MINOR)
  get_llvm_version_component("${LLVM_CMAKELISTS}" LLVM_VERSION_PATCH)
  set (LLVM_VERSION ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH})

  if (MULL_BUILD_32_BITS)
    set (LLVM_BUILD_32_BITS ON CACHE BOOL "Forcing LLVM to be built for 32 bits as well" FORCE)
  endif()
  set (LLVM_ENABLE_PROJECTS "clang" CACHE BOOL "Build only Clang when building against monorepo" FORCE)
  set (LLVM_TARGETS_TO_BUILD "host" CACHE STRING "Do not build targets we cannot JIT" FORCE)

  add_subdirectory(${PATH_TO_LLVM} llvm-build-dir)

  if (NOT TARGET clangTooling)
    message(FATAL_ERROR " 
 Cannot find clangTooling target. Did you forget to clone clang sources?
 Clean CMake cache and make sure they are available at:
 ${PATH_TO_LLVM}/tools/clang")
  endif()

  # Normally, include paths provided by LLVMConfig.cmake
  # In this case we can 'steal' them from real targets
  get_target_property(llvm_support_includes LLVMSupport INCLUDE_DIRECTORIES)
  get_target_property(clang_tooling_includes clangTooling INCLUDE_DIRECTORIES)
  set(LLVM_INCLUDE_DIRS ${llvm_support_includes} ${clang_tooling_includes})
  list(REMOVE_DUPLICATES LLVM_INCLUDE_DIRS)

  get_target_property(clang_bin_directory clang RUNTIME_OUTPUT_DIRECTORY)
  set (MULL_CC ${clang_bin_directory}/clang)
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

set (MULL_CXX_FLAGS "-std=c++11 -fPIC -fvisibility-inlines-hidden -fno-exceptions -Wunused -Werror")
if (NOT LLVM_ENABLE_RTTI)
  set(MULL_CXX_FLAGS "${MULL_CXX_FLAGS} -fno-rtti")
endif()
if (MULL_BUILD_32_BITS)
  set(MULL_CXX_FLAGS "${MULL_CXX_FLAGS} -m32")
endif()

set (MULL_LINK_FLAGS "-rdynamic")
if (MULL_BUILD_32_BITS)
  set(MULL_LINK_FLAGS "${MULL_LINK_FLAGS} -m32")
endif()

add_subdirectory(vendor)

set (MULL_DEFINITIONS ${LLVM_DEFINITIONS})
set (THIRD_PARTY_INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/vendor/LibEBC/lib/include
  ${CMAKE_CURRENT_LIST_DIR}/vendor/libirm/include
  ${CMAKE_SOURCE_DIR}/vendor
)
set (MULL_INCLUDE_DIRS
  ${MULL_SOURCE_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${LLVM_COMPATIBILITY_DIR}
)

if ("LLVM" IN_LIST LLVM_AVAILABLE_LIBS)
  set (MULL_LLVM_COMPATIBILITY_LIBRARIES
    LLVM
  )
else()
  set (MULL_LLVM_COMPATIBILITY_LIBRARIES
    LLVMSupport
    LLVMCore
  )
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/cpack.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/version.cmake)

# Needed by FreeBSD
link_directories(/usr/local/lib)

# https://cmake.org/Wiki/CMake_RPATH_handling#Always_full_RPATH
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" "${LLVM_LIB_PATH}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(LLVMCompatibility/${LLVM_COMPATIBILITY_DIR})
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(tests)
add_subdirectory(tests-lit)
add_subdirectory(vendor/googletest EXCLUDE_FROM_ALL)
add_subdirectory(vendor/libirm EXCLUDE_FROM_ALL)
add_subdirectory(vendor/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(docs)