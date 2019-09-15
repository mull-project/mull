set (SOURCES
  RelationalMutators.cpp
  TrivialCXXMutator.cpp
  )

add_library(mull-cxx-mutators ${SOURCES})
target_link_libraries(mull-cxx-mutators irm)
target_include_directories(mull-cxx-mutators PRIVATE ${MULL_INCLUDE_DIRS})

# TODO: replace with target_compile_options and target_link_options
set_target_properties(mull-cxx-mutators PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
  LINK_FLAGS ${MULL_LINK_FLAGS}
  )