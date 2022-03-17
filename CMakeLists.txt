cmake_minimum_required(VERSION 3.14)

if (NOT MULL_VERSION)
set (MULL_VERSION 0.17.1)
endif()

project(Mull
  LANGUAGES C CXX
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -DRECORD_TIMING=ON
if (RECORD_TIMING)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "\"${CMAKE_COMMAND}\" -E time")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "\"${CMAKE_COMMAND}\" -E time")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_CUSTOM "\"${CMAKE_COMMAND}\" -E time")
endif()

set (PROJECT_VERSION ${MULL_VERSION})

set (PROJECT_DESCRIPTION "Practical mutation testing for C and C++")
set (PROJECT_HOMEPAGE_URL "https://github.com/mull-project/mull")
set (PROJECT_DOCS_URL "https://mull.readthedocs.io")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/properties.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/fixtures.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/functions.cmake)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(LLVM CONFIG REQUIRED)
message("Using LLVM_CONFIG: ${LLVM_CONFIG}")
find_package(Clang CONFIG REQUIRED)
message("Using Clang_CONFIG: ${Clang_CONFIG}")

set (clang_binary_dir ${CLANG_INSTALL_PREFIX})
if (NOT clang_binary_dir)
  # Building against built tree?
  set (clang_binary_dir ${LLVM_BINARY_DIR})
endif()

message("Using LLVM/Clang from: ${clang_binary_dir}")

set(MULL_CC ${clang_binary_dir}/bin/clang)
set(LIT_CLANG ${clang_binary_dir}/bin/clang)
set(LIT_CLANGXX ${clang_binary_dir}/bin/clang++)
set(LIT_LLVM_PROFDATA ${clang_binary_dir}/bin/llvm-profdata)

set(MULL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (MULL_CXX_FLAGS "-fPIC -fvisibility-inlines-hidden -fno-exceptions -Wunused -Wall -fvisibility=hidden")
if (NOT LLVM_ENABLE_RTTI)
  set(MULL_CXX_FLAGS "${MULL_CXX_FLAGS} -fno-rtti")
endif()

add_subdirectory(vendor)

set (MULL_DEFINITIONS ${LLVM_DEFINITIONS})
set (THIRD_PARTY_INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/vendor/libirm/include
  ${CMAKE_SOURCE_DIR}/vendor
)
set (MULL_INCLUDE_DIRS
  ${MULL_SOURCE_DIR}/include
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/cpack.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/version.cmake)

# Needed by FreeBSD
link_directories(/usr/local/lib)

# https://cmake.org/Wiki/CMake_RPATH_handling#Always_full_RPATH
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" "${LLVM_LIB_PATH}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(lib)
add_subdirectory(tools)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(tests-lit)
  add_subdirectory(vendor/googletest EXCLUDE_FROM_ALL)
endif()
add_subdirectory(vendor/libirm EXCLUDE_FROM_ALL)
add_subdirectory(vendor/spdlog EXCLUDE_FROM_ALL)
set(REPROC++ ON)
add_subdirectory(vendor/reproc EXCLUDE_FROM_ALL)
add_subdirectory(docs)
