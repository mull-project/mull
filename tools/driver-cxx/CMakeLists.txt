set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/driver-cxx.cpp
  ${CMAKE_CURRENT_LIST_DIR}/DynamicLibraries.cpp
)

add_mull_executable(
  NAME mull-cxx
  SOURCES ${SOURCES}
  LINK_WITH ebc mull LLVMObject
)