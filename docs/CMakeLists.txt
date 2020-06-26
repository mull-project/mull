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

add_custom_command(
  COMMAND dot -Tsvg db-schema.dot -o generated/db-schema.svg
  OUTPUT generated/db-schema.svg
  DEPENDS db-schema.dot
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(generate-html-docs
  COMMAND make html
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  DEPENDS generated/db-schema.svg
)

add_custom_target(build-docs
  DEPENDS generate-mutators-doc generate-cli-options-doc generate-html-docs
)