add_library(mull-cxx-frontend
  SHARED
  src/ASTInstrumentation.cpp
  src/ASTMutationsSearchVisitor.cpp
  src/ASTMutationPoint.cpp
  src/ASTNodeFactory.cpp
  src/ClangASTMutator.cpp
  src/MullASTMutator.cpp
  src/MullClangPlugin.cpp
  src/MutationMap.cpp

  ${CMAKE_SOURCE_DIR}/lib/AST/ASTConstants.cpp
  ${CMAKE_SOURCE_DIR}/lib/AST/MullClangCompatibility.cpp
)

target_link_libraries(mull-cxx-frontend ${MULL_CXX_LLVM_LIBRARIES})

set_target_properties(mull-cxx-frontend PROPERTIES
  LINK_FLAGS ${MULL_LINK_FLAGS}
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
)
target_include_directories(mull-cxx-frontend PRIVATE
  ${MULL_INCLUDE_DIRS}
  )
target_include_directories(mull-cxx-frontend SYSTEM PRIVATE
  ${THIRD_PARTY_INCLUDE_DIRS}
)

INSTALL(TARGETS mull-cxx-frontend
  RUNTIME DESTINATION lib
)

if (APPLE)
  target_link_libraries(mull-cxx-frontend PRIVATE
    clangAnalysis
    clangAST
    clangASTMatchers
    clangBasic
    clangCodeGen
    clangFrontend
    clangLex
    clangParse
    clangSema
    clangSerialization
    clangEdit
    clangDriver
    clangTooling
  )
else()
  target_link_libraries(mull-cxx-frontend PRIVATE
    clang
  )
endif()
