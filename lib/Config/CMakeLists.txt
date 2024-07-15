add_library(mull-configuration
  Configuration.cpp
  ConfigurationOptions.cpp
  ConfigurationParser.cpp
)
target_include_directories(mull-configuration PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(mull-configuration PRIVATE ${LLVM_INCLUDE_DIRS})

# FIXME: replace with target_compile_options()
set_target_properties(mull-configuration PROPERTIES COMPILE_FLAGS ${MULL_CXX_FLAGS})

add_custom_command(
  OUTPUT
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedYamlMapping.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedConfiguration.h
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedDebugConfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedParallelizationConfig.h
  COMMAND
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/config_gen.py ${CMAKE_SOURCE_DIR}/include/mull/Config/config.json ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_SOURCE_DIR}/include/mull/Config/config.json ${CMAKE_CURRENT_SOURCE_DIR}/config_gen.py
)
target_include_directories(mull-configuration PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(generate-config-sources DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/GeneratedYamlMapping.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedConfiguration.h
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedDebugConfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedParallelizationConfig.h)
add_dependencies(mull-configuration generate-config-sources)
