add_library(Diagnostics
  Diagnostics.cpp
  ${CMAKE_SOURCE_DIR}/include/mull/Diagnostics/Diagnostics.h
)
target_include_directories(Diagnostics PUBLIC ${CMAKE_SOURCE_DIR}/include)

set_target_properties(Diagnostics PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
  LINK_FLAGS ${MULL_LINK_FLAGS}
)