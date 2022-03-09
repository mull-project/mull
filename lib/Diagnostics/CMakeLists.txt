add_library(mull-diagnostics
  Diagnostics.cpp
  ${CMAKE_SOURCE_DIR}/include/mull/Diagnostics/Diagnostics.h
)
target_link_libraries(mull-diagnostics spdlog)
