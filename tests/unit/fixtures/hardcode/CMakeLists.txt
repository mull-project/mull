set (HARDCODED_FILES
  ${CMAKE_CURRENT_LIST_DIR}/APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator.ll
  ${CMAKE_CURRENT_LIST_DIR}/fixture_simple_test_tester_module.bc
)

foreach(file ${HARDCODED_FILES})
  add_fixture(${file})
endforeach()


