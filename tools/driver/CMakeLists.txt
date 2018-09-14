
add_executable(mull-driver driver.cpp)

target_link_libraries(mull-driver
  mull
)

set(mulldriver_rpath "") # to be filled later.

# compile flags
get_target_property(default_compile_flags mull-driver COMPILE_FLAGS)
if(NOT default_compile_flags)
  set(default_compile_flags "")
endif()
set(mulldriver_compileflags ${default_compile_flags} ${LLVM_CXX_FLAGS})
set_target_properties(mull-driver
  PROPERTIES COMPILE_FLAGS
  "${mulldriver_compileflags}"
)

# Link flags
get_target_property(default_link_flags mull-driver LINK_FLAGS)
if(NOT ${default_link_flags})
set(default_link_flags "")
endif()
set(mull_driver_link_flags
  "${default_link_flags} ${LLVM_LINK_FLAGS} -rdynamic -pthread"
)
set_target_properties(mull-driver PROPERTIES LINK_FLAGS "${mull_driver_link_flags}")

# rpath
get_target_property(default_rpath mull-driver INSTALL_RPATH)
set(mulldriver_rpath ${default_rpath} ${mulldriver_rpath})
set_target_properties(mull-driver
  PROPERTIES INSTALL_RPATH
  "${mulldriver_rpath}"
)

INSTALL(TARGETS mull-driver
  RUNTIME DESTINATION bin
)
