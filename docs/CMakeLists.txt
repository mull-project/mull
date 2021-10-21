file(MAKE_DIRECTORY generated)
file(MAKE_DIRECTORY command-line/generated)

add_custom_target(generate-mull-cxx-cli-options-doc
  COMMAND $<TARGET_FILE:mull-cxx-${LLVM_VERSION_MAJOR}> --dump-cli > command-line/generated/mull-cxx-cli-options.rst
  DEPENDS mull-cxx
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(generate-mull-runner-cli-options-doc
  COMMAND $<TARGET_FILE:mull-runner-${LLVM_VERSION_MAJOR}> --dump-cli > command-line/generated/mull-runner-cli-options.rst
  DEPENDS mull-runner
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )

add_custom_target(generate-mutators-doc
  COMMAND $<TARGET_FILE:mull-cxx-${LLVM_VERSION_MAJOR}> --dump-mutators > generated/Mutators.rst
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
  DEPENDS
    generate-mutators-doc
    generate-mull-cxx-cli-options-doc
    generate-mull-runner-cli-options-doc
    generate-html-docs
)