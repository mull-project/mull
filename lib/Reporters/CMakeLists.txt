add_library(mull-reporters
  SQLiteReporter.cpp
  PatchesReporter.cpp
  GithubAnnotationsReporter.cpp
  IDEReporter.cpp
  MutationTestingElementsReporter.cpp
)
find_package(SQLite3 REQUIRED)
target_link_libraries(mull-reporters PRIVATE SQLite::SQLite3 json11)
