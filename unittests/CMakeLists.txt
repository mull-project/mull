add_custom_target(MutangUnitTests)
set_target_properties(MutangUnitTests PROPERTIES FOLDER "Tests")

function(add_mutang_unittest test_dirname)
  add_unittest(MutangUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(Context)
add_subdirectory(Laboratory)
add_subdirectory(MutationEngine)
add_subdirectory(MutationOperators)
add_subdirectory(TestFinders)

