function(detect_compiler)
  cmake_parse_arguments(DETECT_COMPILER "" "EXTENSION;COMPILER" "" ${ARGN})

  string(TOLOWER ${DETECT_COMPILER_EXTENSION} extension)
  string(COMPARE EQUAL ${extension} ".c" is_c_source_file)
  string(COMPARE EQUAL ${extension} ".cpp" is_cpp_source_file)
  if (${is_c_source_file})
    set(${DETECT_COMPILER_COMPILER} ${LLVM_CC} PARENT_SCOPE)
  elseif(${is_cpp_source_file})
    set(${DETECT_COMPILER_COMPILER} ${LLVM_CXX} PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Could not detect compiler for '${DETECT_COMPILER_EXTENSION}'")
  endif()
endfunction()

function(generate_fixtures)
  cmake_parse_arguments(GENERATE_BITCODE "" "TARGET" "SOURCES" ${ARGN})

  add_custom_target(${GENERATE_BITCODE_TARGET})

  foreach(input ${GENERATE_BITCODE_SOURCES})
    get_filename_component(input_filename ${input} NAME_WE)
    get_filename_component(input_extension ${input} EXT)
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${input}.bc")

    set(compiler "")
    detect_compiler(
      EXTENSION ${input_extension}
      COMPILER compiler
    )

    add_custom_command(
      OUTPUT ${output}
      COMMAND ${compiler} -c -emit-llvm ${input} -o ${output}
      DEPENDS ${input}
    )

    set(bitcode_target "generate-${GENERATE_BITCODE_TARGET}-${input_filename}-bitcode")
    add_custom_target(${bitcode_target}
      DEPENDS ${output}
    )

    add_dependencies(${GENERATE_BITCODE_TARGET} ${bitcode_target})
  endforeach()
endfunction()

