add_library(mull-configuration
  Configuration.cpp
  ConfigurationOptions.cpp
  ConfigurationParser.cpp
)
target_include_directories(mull-configuration PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(mull-configuration PRIVATE ${LLVM_INCLUDE_DIRS})
# FIXME: replace with target_compile_options()
set_target_properties(mull-configuration PROPERTIES COMPILE_FLAGS ${MULL_CXX_FLAGS})