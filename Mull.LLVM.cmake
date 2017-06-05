cmake_minimum_required(VERSION 3.5.1)

if (NOT LLVM_ROOT)
  message(FATAL_ERROR "LLVM_ROOT variable is not defined!")
endif()

set(LLVM_CONFIG_PATH "${LLVM_ROOT}/bin/llvm-config")

# Getting LLVM CXX flags using llvm-config
function(llvm_get_cxx_flags OUT_CXX_FLAGS)
  set(config_command ${LLVM_CONFIG_PATH}
    "--cxxflags"
  )
  execute_process(
    COMMAND ${config_command}
    RESULT_VARIABLE HAD_ERROR
    OUTPUT_VARIABLE config_output
  )
  if(NOT HAD_ERROR)
    # Remove -O3 or any other -O? flags
    string(REPLACE "-O? " "" config_output "${config_output}")
    # Remove DNDEBUG to not disable assertions.
    string(REPLACE "-DNDEBUG " "" config_output "${config_output}")
    set(llvm_cxx_flags ${config_output})
  else()
    string(REPLACE ";" " " config_command_str "${config_command}")
  endif()
  set(${OUT_CXX_FLAGS} ${llvm_cxx_flags} PARENT_SCOPE)
endfunction()

# Getting LLVM linker flags using llvm-config
function(llvm_get_link_flags OUT_LINK_FLAGS)
  set(config_command ${LLVM_CONFIG_PATH}
    "--ldflags"
  )
  execute_process(
    COMMAND ${config_command}
    RESULT_VARIABLE HAD_ERROR
    OUTPUT_VARIABLE config_output
  )
  if(NOT HAD_ERROR)
    string(REGEX REPLACE "[\r\n(\r\n)]+" "" config_output ${config_output})
    set(llvm_link_flags ${config_output})
  else()
    string(REPLACE ";" " " config_command_str "${config_command}")
    message(STATUS "${config_command_str}")
    message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
  endif()

  set(${OUT_LINK_FLAGS} ${llvm_link_flags} PARENT_SCOPE)
endfunction()

# Getting LLVM lib directory using llvm-config.
function(llvm_get_lib_dir OUT_LIB_DIR)
  set(config_command ${LLVM_CONFIG_PATH}
    "--libdir"
  )
  execute_process(
    COMMAND ${config_command}
    RESULT_VARIABLE HAD_ERROR
    OUTPUT_VARIABLE config_output
  )

  if(NOT HAD_ERROR)
    string(REGEX REPLACE "[\r\n(\r\n)]+" "" config_output ${config_output})
    set(llvm_lib_dir ${config_output})
  else()
    string(REPLACE ";" " " config_command_str "${config_command}")
    message(STATUS "${config_command_str}")
    message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
  endif()

  set(${OUT_LIB_DIR} ${llvm_lib_dir} PARENT_SCOPE)
endfunction()

# Getting LLVM libraries
function(llvm_get_libs OUT_LIBS components)
  set(config_command ${LLVM_CONFIG_PATH}
    "--libs"
    ${components}
  )
  execute_process(
    COMMAND ${config_command}
    RESULT_VARIABLE HAD_ERROR
    OUTPUT_VARIABLE config_output
  )
  if(NOT HAD_ERROR)
    string(REGEX REPLACE "[ \t\r\n(\r\n)]+" ";" config_output ${config_output})
    set(llvm_libs "${config_output}")
  else()
    string(REPLACE ";" " " config_command_str "${config_command}")
  endif()
  set(${OUT_LIBS} ${llvm_libs} PARENT_SCOPE)
endfunction()
