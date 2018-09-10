add_executable(mull-driver driver.cpp)
target_link_libraries(mull-driver
  mull
)

set_target_properties(mull-driver PROPERTIES
  LINK_FLAGS ${MULL_LINK_FLAGS}
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
target_include_directories(mull-driver PUBLIC
  ${MULL_INCLUDE_DIRS}
)

INSTALL(TARGETS mull-driver
  RUNTIME DESTINATION bin
)
