add_library(mull-source-manager
  SourceCodeReader.cpp
  SourceManager.cpp
)
target_link_libraries(mull-source-manager mull-core)
