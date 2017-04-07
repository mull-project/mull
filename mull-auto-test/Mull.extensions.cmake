
message(STATUS "Mull> Mull CMake Extensions file is included")

function(mull_clone_target existing_target new_target)
  message(STATUS
    "Mull.mull_clone_target()> cloning target '${existing_target}' -> '${new_target}'...")

  get_target_property(existing_target_source_dir ${existing_target} SOURCE_DIR)
  get_target_property(existing_target_sources ${existing_target} SOURCES)
  get_target_property(existing_target_include_directories ${existing_target} INCLUDE_DIRECTORIES)
  get_target_property(existing_target_link_libraries ${existing_target} LINK_LIBRARIES)
  get_target_property(existing_target_install_rpath ${existing_target} INSTALL_RPATH)

  if(NOT existing_target_link_libraries)
    set(existing_target_link_libraries "")
  endif()

  set(new_target_sources "")
  foreach(existing_target_source_file ${existing_target_sources})
      if (existing_target_source_file MATCHES "^\/")
        set(new_target_source_file
          "${existing_target_source_file}")
      else()
        set(new_target_source_file
          "${existing_target_source_dir}/${existing_target_source_file}")
      endif()

      list(APPEND new_target_sources ${new_target_source_file})
  endforeach()

  add_library(${new_target} SHARED ${new_target_sources})

  set_target_properties(${new_target}
    PROPERTIES
    INCLUDE_DIRECTORIES "${existing_target_include_directories}"
    INSTALL_RPATH "${existing_target_install_rpath}"
  )

  target_link_libraries(${new_target} ${existing_target_link_libraries})

endfunction(mull_clone_target)

function(mull_clone_executable_target existing_target new_target)
  message(STATUS
    "Mull.mull_clone_executable_target()> cloning target '${existing_target}' -> '${new_target}'...")

  get_target_property(existing_target_source_dir ${existing_target} SOURCE_DIR)
  get_target_property(existing_target_sources ${existing_target} SOURCES)
  get_target_property(existing_target_include_directories ${existing_target} INCLUDE_DIRECTORIES)
  get_target_property(existing_target_link_libraries ${existing_target} LINK_LIBRARIES)
  get_target_property(existing_target_install_rpath ${existing_target} INSTALL_RPATH)

  set(new_target_sources "")
  foreach(existing_target_source_file ${existing_target_sources})
      if (existing_target_source_file MATCHES "^\/")
        set(new_target_source_file
          "${existing_target_source_file}")
      else()
        set(new_target_source_file
          "${existing_target_source_dir}/${existing_target_source_file}")
      endif()

      list(APPEND new_target_sources ${new_target_source_file})
  endforeach()

  add_executable(${new_target} ${new_target_sources})

  set_target_properties(${new_target}
    PROPERTIES INCLUDE_DIRECTORIES "${existing_target_include_directories}")

  set_target_properties(${new_target}
    PROPERTIES
    INCLUDE_DIRECTORIES "${existing_target_include_directories}"
    INSTALL_RPATH "${existing_target_install_rpath}"
  )

  target_link_libraries(${new_target} ${existing_target_link_libraries})
endfunction(mull_clone_executable_target)
