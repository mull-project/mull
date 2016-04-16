configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

set(MUTANG_TEST_DEPENDS
        FileCheck
        mutang-lab
        )

add_custom_target(mutang-test-depends DEPENDS ${MUTANG_TEST_DEPENDS})

add_lit_testsuite(check-mutang "Running the Mutang Integration Tests"
  ${CMAKE_CURRENT_BINARY_DIR} 
  PARAMS  llvm_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  DEPENDS ${MUTANG_TEST_DEPENDS}
)
set_target_properties(check-mutang PROPERTIES FOLDER "Tests")

