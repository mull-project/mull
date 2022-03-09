add_library(mull-filters
  JunkMutationFilter.cpp
  NoDebugInfoFilter.cpp
  BlockAddressFunctionFilter.cpp
  VariadicFunctionFilter.cpp
  FilePathFilter.cpp
  GitDiffReader.cpp
  GitDiffFilter.cpp
  Filters.cpp
  CoverageFilter.cpp
)
target_link_libraries(mull-filters mull-core)
