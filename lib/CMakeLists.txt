# MESSAGE( STATUS "CMAKE_BINARY_DIR:         " ${file(GLOB tds *.td)} )

llvm_add_library(mutang 
  Laboratory.cpp

  ADDITIONAL_HEADER_DIRS
  ${MUTANG_MAIN_INCLUDE_DIR}
)
