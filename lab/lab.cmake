set (FIXTURES_STORAGE_KEY "FIXTURES_STORAGE_KEY")

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

function(add_fixtures_target target)
  add_custom_target(fixtures)
  define_property(TARGET
    PROPERTY ${FIXTURES_STORAGE_KEY}
    BRIEF_DOCS "Bla"
    FULL_DOCS "Bla bla"
  )
endfunction()

function(generate_fixtures)
  cmake_parse_arguments(GENERATE_BITCODE "" "TARGET" "SOURCES" ${ARGN})

  add_custom_target(${GENERATE_BITCODE_TARGET})

  set (outputs "")

  foreach(input ${GENERATE_BITCODE_SOURCES})
    get_filename_component(input_filename ${input} NAME_WE)
    get_filename_component(input_extension ${input} EXT)
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${input_filename}.bc")

    list(APPEND outputs ${output})

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

  set_property(TARGET ${GENERATE_BITCODE_TARGET}
    PROPERTY ${FIXTURES_STORAGE_KEY}
    ${outputs}
  )
endfunction()

function(generate_fixtures_cpp target)
  set(fixtures_cpp_file ${CMAKE_CURRENT_BINARY_DIR}/FixturesFactory.h)

  file(WRITE ${fixtures_cpp_file}
"#include \"ModuleLoader.h\"
#include \"MullModule.h\"

#include <string>

using namespace llvm;
using namespace mull;

class FixturesFactory {
  ModuleLoader &loader;
public:
  FixturesFactory(ModuleLoader &loader) : loader(loader) {}
"
)

  set(prefix ${CMAKE_CURRENT_BINARY_DIR})

  get_property(dependencies
    TARGET ${target}
    PROPERTY MANUALLY_ADDED_DEPENDENCIES
  )


  foreach(fixture_target ${dependencies})
    get_property(bitcode_files
      TARGET ${fixture_target}
      PROPERTY ${FIXTURES_STORAGE_KEY}
    )
    foreach(bitcode_file ${bitcode_files})
      string(REPLACE ${prefix} "" safe_filename ${bitcode_file})
      string(REGEX REPLACE "/|\\." "_" safe_filename ${safe_filename})
      message("${safe_filename} -> ${bitcode_file}")

      file(APPEND ${fixtures_cpp_file} "
  std::unique_ptr<MullModule> create${safe_filename}() {
    return loader.loadModuleAtPath(\"${bitcode_file}\");
  }
")
    endforeach()
  endforeach()

  file(APPEND ${fixtures_cpp_file} "};")

  message(${fixtures_cpp_file})
endfunction()

