set (CPACK_GENERATOR DEB)

set (CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
set (CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})

if (NOT NO_PRECOMPILED_LLVM)
  set (deps "zlib1g, libncurses5, libstdc++6, libxml2, libsqlite3-0")
else()
  set (deps "libllvm${LLVM_VERSION_MAJOR}, libxml2, libsqlite3-0")
endif()

set (CPACK_DEBIAN_PACKAGE_DEPENDS ${deps})
