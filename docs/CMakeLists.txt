file(MAKE_DIRECTORY generated)
file(MAKE_DIRECTORY command-line/generated)

add_custom_target(generate-mull-runner-cli-options-doc
  COMMAND $<TARGET_FILE:mull-runner-${LLVM_VERSION_MAJOR}> --dump-cli > command-line/generated/mull-runner-cli-options.rst
  DEPENDS mull-runner-${LLVM_VERSION_MAJOR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )

add_custom_target(generate-mutators-doc
  COMMAND $<TARGET_FILE:mull-runner-${LLVM_VERSION_MAJOR}> --dump-mutators > generated/Mutators.rst
  DEPENDS mull-runner-${LLVM_VERSION_MAJOR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(generate-html-docs
  COMMAND make html
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(build-docs
  DEPENDS
    generate-mutators-doc
    generate-mull-runner-cli-options-doc
    generate-html-docs
)
