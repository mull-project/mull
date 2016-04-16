add_custom_target(MutangUnitTests)
set_target_properties(MutangUnitTests PROPERTIES FOLDER "Tests")

function(add_mutang_unittest test_dirname)
        add_unittest(MutangUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(Laboratory)

