
message(STATUS "Mull> Mull CMake Extensions file is included")

function(mull_clone_target existing_target new_target)
  message(STATUS
    "Mull.mull_clone_target()> cloning target '${existing_target}' -> '${new_target}'...")

  get_target_property(existing_target_type ${existing_target} TYPE)
  get_target_property(existing_target_source_dir ${existing_target} SOURCE_DIR)
  get_target_property(existing_target_sources ${existing_target} SOURCES)
  get_target_property(existing_target_include_directories ${existing_target} INCLUDE_DIRECTORIES)
  get_target_property(existing_target_link_libraries ${existing_target} LINK_LIBRARIES)
  get_target_property(existing_target_install_rpath ${existing_target} INSTALL_RPATH)
  get_target_property(existing_target_compile_definitions ${existing_target} COMPILE_DEFINITIONS)

  if(NOT existing_target_link_libraries)
    set(existing_target_link_libraries "")
  endif()
  if(NOT existing_target_include_directories)
    set(existing_target_include_directories "")
  endif()
  if(NOT existing_target_compile_definitions)
    set(existing_target_compile_definitions "")
  endif()

  set(new_target_sources "")
  foreach(existing_target_source_file ${existing_target_sources})
      if (existing_target_source_file MATCHES "^\\/")
        set(new_target_source_file
          "${existing_target_source_file}")
      else()
        set(new_target_source_file
          "${existing_target_source_dir}/${existing_target_source_file}")
      endif()

      list(APPEND new_target_sources ${new_target_source_file})
  endforeach()

  if(existing_target_type STREQUAL SHARED_LIBRARY)
    add_library(${new_target} SHARED ${new_target_sources})
  elseif(existing_target_type STREQUAL EXECUTABLE)
    add_executable(${new_target} ${new_target_sources})
  elseif(existing_target_type STREQUAL STATIC_LIBRARY)
    add_library(${new_target} STATIC ${new_target_sources})
  else()
    message(FATAL_ERROR "Type: ${existing_target_type}")
  endif()

  set_target_properties(${new_target}
    PROPERTIES
    COMPILE_DEFINITIONS "${existing_target_compile_definitions}"
    INCLUDE_DIRECTORIES "${existing_target_include_directories}"
    INSTALL_RPATH "${existing_target_install_rpath}"
  )

  target_link_libraries(${new_target} ${existing_target_link_libraries})

endfunction(mull_clone_target)
