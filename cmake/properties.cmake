define_property(GLOBAL
                PROPERTY TEST_FIXTURES
                BRIEF_DOCS "Holds list if all generated fixtures"
                FULL_DOCS "See above")

define_property(GLOBAL
  PROPERTY TEST_FIXTURES_DEPENDENCIES
                BRIEF_DOCS "Holds list if build targets for fixtures"
                FULL_DOCS "See above")

function(add_fixture fixture)
  set_property(GLOBAL APPEND PROPERTY TEST_FIXTURES ${fixture})
endfunction()

function(add_fixture_dependency dependency)
  set_property(GLOBAL APPEND PROPERTY TEST_FIXTURES_DEPENDENCIES ${dependency})
endfunction()

