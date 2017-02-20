
add_executable(mull-driver driver.cpp)

target_link_libraries(mull-driver
  mull
)

set(mulldriver_rpath "") # to be filled later.

if(MULL_SUPPORT_RUST)
  set(mulldriver_rpath
    ${mulldriver_rpath} ${MULL_RUST_LIB_PATH}
  )
endif()

# rpath
get_target_property(default_rpath mull-driver INSTALL_RPATH)
set(mulldriver_rpath ${default_rpath} ${mulldriver_rpath})
set_target_properties(mull-driver
  PROPERTIES INSTALL_RPATH
  "${mulldriver_rpath}"
)
