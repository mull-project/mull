cmake_minimum_required(VERSION 3.5.1)

project(Mull)
enable_language(C CXX)

if (NOT LLVM_ROOT)
  message(FATAL_ERROR "This CMakeLists.txt file expects cmake to be called with
  LLVM_ROOT provided:\n \
  -DLLVM_ROOT=path-to-llvm-installation")
endif()

# This enables assertions for Release builds.
# https://stackoverflow.com/questions/22140520/how-to-enable-assert-in-cmake-release-mode
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set (search_paths
  ${LLVM_ROOT}
  ${LLVM_ROOT}/lib/cmake
  ${LLVM_ROOT}/lib/cmake/llvm
  ${LLVM_ROOT}/lib/cmake/clang
  ${LLVM_ROOT}/share/clang/cmake/
  ${LLVM_ROOT}/share/llvm/cmake/
)

find_package(LLVM REQUIRED CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)
find_package(Clang REQUIRED CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)

set (llvm_patch_version "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}")
set (llvm_minor_version "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.x")
set (llvm_major_version "${LLVM_VERSION_MAJOR}.x.x")

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${llvm_patch_version})
  set (LLVM_COMPATIBILITY_DIR ${llvm_patch_version})

elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${llvm_minor_version})
  set (LLVM_COMPATIBILITY_DIR ${llvm_minor_version})

elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${llvm_major_version})
  set (LLVM_COMPATIBILITY_DIR ${llvm_major_version})
else()
  message(FATAL_ERROR "LLVM-${LLVM_PACKAGE_VERSION} is not supported")
endif()

set(MULL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (MULL_CXX_FLAGS "-std=c++11")
if (NOT LLVM_ENABLE_RTTI)
  set(MULL_CXX_FLAGS "${MULL_CXX_FLAGS} -fno-rtti")
endif()

set (MULL_LINK_FLAGS "-rdynamic")
set (MULL_DEFINITIONS ${LLVM_DEFINITIONS})
set (MULL_INCLUDE_DIRS
  ${MULL_SOURCE_DIR}/include
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/LLVMCompatibility/${LLVM_COMPATIBILITY_DIR}
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

