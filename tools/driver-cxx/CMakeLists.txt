set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/driver-cxx.cpp
  ${CMAKE_CURRENT_LIST_DIR}/DynamicLibraries.cpp
)

if ("LLVM" IN_LIST LLVM_AVAILABLE_LIBS)
  set (MULL_CXX_LLVM_LIBRARIES
    LLVM
  )
else()
  set (MULL_CXX_LLVM_LIBRARIES
    LLVMObject
  )
endif()

add_mull_executable(
  NAME mull-cxx
  SOURCES ${SOURCES}
  LINK_WITH ebc mull ${MULL_CXX_LLVM_LIBRARIES} json11
)
