add_custom_target(generate-cli-options-doc
  COMMAND $<TARGET_FILE:mull-cxx> --dump-cli > generated/CLIOptions.rst
  DEPENDS mull-cxx
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(generate-mutators-doc
  COMMAND $<TARGET_FILE:mull-cxx> --dump-mutators > generated/Mutators.rst
  DEPENDS mull-cxx
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(build-docs
  DEPENDS generate-mutators-doc generate-cli-options-doc
)