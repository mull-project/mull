set (CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set (CPACK_PACKAGE_VENDOR mull-project)
set (CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set (CPACK_PACKAGE_CONTACT "Alex Denisov <alex@lowlevelbits.org>")
set (CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})

set (CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
configure_file(${CMAKE_SOURCE_DIR}/LICENSE ${CPACK_RESOURCE_FILE_LICENSE})

set (CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/cmake/packaging/ReadMe.txt")
set (CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/cmake/packaging/Welcome.txt")

if (APPLE)
  include(cmake/packaging/cpack.PackageMaker.cmake)
else()
  if(EXISTS /etc/os-release)
    file(READ /etc/os-release OS_RELEASE)
    string(REGEX MATCH "ID=([A-Za-z]+)" match ${OS_RELEASE}})
    if (NOT match)
      message(WARNING "Cannot find distro ID in the /etc/os-release")
      return()
    endif()

    set (CPACK_SYSTEM_NAME ${CMAKE_MATCH_1})

    if (${CPACK_SYSTEM_NAME} STREQUAL "debian")
      include(cmake/packaging/cpack.DEB-debian.cmake)
    elseif(${CPACK_SYSTEM_NAME} STREQUAL "ubuntu")
      include(cmake/packaging/cpack.DEB-ubuntu.cmake)
    elseif(${CPACK_SYSTEM_NAME} STREQUAL "fedora")
      include(cmake/packaging/cpack.RPM-fedora.cmake)
    else()
      message(WARNING "Mull cannot generate package for ${CPACK_SYSTEM_NAME} yet.")
    endif()

  endif()
endif()

include(CPack)

