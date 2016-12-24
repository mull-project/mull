configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

set(MULL_TEST_DEPENDS
        FileCheck
        mull-driver
        )

add_custom_target(mull-test-depends DEPENDS ${MULL_TEST_DEPENDS})

add_lit_testsuite(check-mull "Running the Mull Integration Tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS  llvm_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  DEPENDS ${MULL_TEST_DEPENDS}
)
set_target_properties(check-mull PROPERTIES FOLDER "Tests")

