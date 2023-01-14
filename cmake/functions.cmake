macro(add_mull_executable)
  set (prefix local)
  set (optionArguments )
  set (singleValueArguments NAME)
  set (multipleValueArguments SOURCES LINK_WITH)

  cmake_parse_arguments(${prefix}
    "${optionArguments}"
    "${singleValueArguments}"
    "${multipleValueArguments}"
    ${ARGN}
  )

  add_executable(${local_NAME} ${local_SOURCES})
  target_link_libraries(${local_NAME} ${local_LINK_WITH} MullVersion)

  set_target_properties(${local_NAME} PROPERTIES
    COMPILE_FLAGS ${MULL_CXX_FLAGS}
  )
  target_include_directories(${local_NAME} PUBLIC
    ${MULL_INCLUDE_DIRS}
  )
  target_include_directories(${local_NAME} SYSTEM PRIVATE
    ${THIRD_PARTY_INCLUDE_DIRS}
  )

  INSTALL(TARGETS ${local_NAME}
    RUNTIME DESTINATION bin
  )
endmacro()

function(determine_clang_component_lib lib_out component)
  # Assume the component comes as a separate lib
  # until otherwise proven
  set(${lib_out} ${component} PARENT_SCOPE)

  # Check whether the LLVM shared library was built
  if("${LLVM_LINK_LLVM_DYLIB}" STREQUAL "ON" AND NOT "${LLVM_DYLIB_COMPONENTS}" STREQUAL "")
    # The LLVM_DYLIB_COMPONENTS variable lists the components included in the
    # shared library. According to https://llvm.org/docs/BuildingADistribution.html,
    # it is a list of semi-colon separated names of LLVM components or special
    # purpose names. The latter are
    #
    # 1. all - All available LLVM component libraries included
    # 2. Native - The LLVM target for the native system
    # 3. AllTargetsAsmParsers - All included target ASM parser libraries
    # 4. AllTargetsDescs - All included target description libraries
    # 5. AllTargetsDisassemblers - All included target dissassembler libraries
    # 6. AllTargetsInfos - All included target info libraries
    #
    # I can't find any relevant reference to Native in the LLVM source tree
    # so we leave that out for now
    if("${LLVM_DYLIB_COMPONENTS}" STREQUAL "all")
      # Dylib includes all components, linking against it is a safe bet
      set(${lib_out} clang-cpp PARENT_SCOPE)
    elseif(${component} IN_LIST LLVM_DYLIB_COMPONENTS)
      # Component listed in DYLIB_COMPONENTS, included
      set(${lib_out} clang-cpp PARENT_SCOPE)
    else()
      # Look through remaining special purpose names. Based on llvm/cmake/modules/LLVM-Config.cmake
      # in the LLVM repo, the targets correspond to the following
      #
      # AllTargetsAsmParsers - All components ending in AsmParser
      # AllTargetsDescs - All components ending in Desc
      # AllTargetsDisassemblers - All components ending in Disassembler
      # AllTargetsInfos - All components ending in Info
      set(suffixes AsmParser Desc Disassembler Info)

      foreach(s ${suffixes})
        if(${component} MATCHES ".*${s}$" AND "${LLVM_DYLIB_COMPONENTS}" STREQUAL "AllTargets${s}s")
          set(${lib_out} clang-cpp PARENT_SCOPE)
        endif()
      endforeach()
    endif()
  endif()
endfunction()
