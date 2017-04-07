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

get_target_property(mull_autotest_driver_link_flags mull-autotest-driver LINK_FLAGS)
if(NOT mull_autotest_driver_link_flags)
  set(mull_autotest_driver_link_flags "")
endif()
set_target_properties(mull-autotest-driver
  PROPERTIES
  LINK_FLAGS
  "${mull_autotest_driver_link_flags} -force_load ${MULL_DEPENDENCY_LLVMAsmParser}")
