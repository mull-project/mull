add_library(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} SHARED mull-cxx-ir-frontend.cpp)
target_link_libraries(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PRIVATE LLVM mull)
target_include_directories(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PRIVATE ${MULL_INCLUDE_DIRS})
target_include_directories(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES COMPILE_FLAGS ${MULL_CXX_FLAGS})
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES LIBRARY_OUTPUT_NAME mull-ir-frontend-${LLVM_VERSION_MAJOR})
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES PREFIX "")
set_target_properties(mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR} PROPERTIES SUFFIX "")
install(TARGETS mull-cxx-ir-frontend-${LLVM_VERSION_MAJOR}
  RUNTIME DESTINATION lib
)