set (CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME}-${LLVM_VERSION_MAJOR})
set (CPACK_PACKAGE_VENDOR mull-project)
set (CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set (CPACK_PACKAGE_CONTACT "Alex Denisov <alex@lowlevelbits.org>")
set (CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})

set (CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
configure_file(${CMAKE_SOURCE_DIR}/LICENSE ${CPACK_RESOURCE_FILE_LICENSE})

set (CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/cmake/packaging/ReadMe.txt")
set (CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/cmake/packaging/Welcome.txt")

if (APPLE)
  set (CPACK_SYSTEM_NAME "macOS")
  execute_process(
    COMMAND sw_vers -productVersion
    OUTPUT_VARIABLE CPACK_SYSTEM_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
  set (CPACK_SYSTEM_NAME "FreeBSD")
  execute_process(
    COMMAND freebsd-version
    OUTPUT_VARIABLE CPACK_SYSTEM_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

else()
  if(EXISTS /etc/os-release)
    file(READ /etc/os-release OS_RELEASE)
    string(REGEX REPLACE "\r?\n" ";" LINES "${OS_RELEASE}")

    foreach(LINE ${LINES})
      string(REGEX MATCH "^ID=\"?([A-Za-z0-9._-]+)\"?" match ${LINE}})
      if (match)
        set (CPACK_SYSTEM_NAME ${CMAKE_MATCH_1})
      endif()

      string(REGEX MATCH "^VERSION_ID=\"?([A-Za-z0-9._-]+)\"?" match ${LINE}})
      if (match)
        set (CPACK_SYSTEM_VERSION ${CMAKE_MATCH_1})
      endif()
    endforeach()

  else()
    set (CPACK_SYSTEM_NAME "unknown")

  endif()
endif()

if (${CPACK_SYSTEM_NAME} STREQUAL "macOS")
  include(cmake/packaging/cpack.ZIP-macOS.cmake)
elseif (${CPACK_SYSTEM_NAME} STREQUAL "debian")
  include(cmake/packaging/cpack.DEB-debian.cmake)
elseif(${CPACK_SYSTEM_NAME} STREQUAL "ubuntu")
  include(cmake/packaging/cpack.DEB-ubuntu.cmake)
elseif(${CPACK_SYSTEM_NAME} STREQUAL "fedora")
  include(cmake/packaging/cpack.RPM-fedora.cmake)
elseif(${CPACK_SYSTEM_NAME} STREQUAL "FreeBSD")
  include(cmake/packaging/cpack.PKG-freebsd.cmake)
else()
  message(WARNING "Mull cannot generate package for ${CPACK_SYSTEM_NAME} yet.")
endif()

set (MULL_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-LLVM-${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}-${CPACK_SYSTEM_NAME}-${CPACK_SYSTEM_VERSION}")

set (CPACK_PACKAGE_FILE_NAME ${MULL_PACKAGE_FILE_NAME})

set (CPACK_STRIP_FILES TRUE)

include(CPack)

file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/PACKAGE_FILE_NAME ${MULL_PACKAGE_FILE_NAME})
