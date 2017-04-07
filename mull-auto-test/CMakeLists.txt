include("Mull.extensions.cmake")

mull_clone_target(mull mull-bitcode)
set_target_properties(mull-bitcode PROPERTIES XCODE_ATTRIBUTE_CC "${MULL_CC}")
set_target_properties(mull-bitcode PROPERTIES XCODE_ATTRIBUTE_CXX "${MULL_CXX}")

mull_clone_target(MullUnitTests mull-unittests-bitcode)
set_target_properties(mull-unittests-bitcode PROPERTIES XCODE_ATTRIBUTE_CC "${MULL_CC}")
set_target_properties(mull-unittests-bitcode PROPERTIES XCODE_ATTRIBUTE_CXX "${MULL_CXX}")

mull_clone_target(google-test google-test-bitcode)
set_target_properties(google-test-bitcode PROPERTIES XCODE_ATTRIBUTE_CC "${MULL_CC}")
set_target_properties(google-test-bitcode PROPERTIES XCODE_ATTRIBUTE_CXX "${MULL_CXX}")

mull_clone_executable_target(mull-driver mull-autotest-driver)

add_dependencies(mull-autotest-driver
  mull-bitcode
  mull-unittests-bitcode
  google-test-bitcode
)

# We need to force_load LLVMAsmParser so that its API is available for execution
# by JIT'ed Mull's unit tests.
get_target_property(mull_autotest_driver_link_flags mull-autotest-driver LINK_FLAGS)
if(NOT mull_autotest_driver_link_flags)
  set(mull_autotest_driver_link_flags "")
endif()
set_target_properties(mull-autotest-driver
  PROPERTIES
  LINK_FLAGS
  "${mull_autotest_driver_link_flags} -force_load ${MULL_DEPENDENCY_LLVMAsmParser}")

# This copies (symlinks) fixtures/*.bc files that are used by unit tests.
add_custom_command(TARGET mull-autotest-driver
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${MULL_SOURCE_DIR}/unittests/fixtures
    $<TARGET_FILE_DIR:mull-autotest-driver>/fixtures)
