add_library(mull-cxx-frontend-${LLVM_VERSION_MAJOR}
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

target_link_libraries(mull-cxx-frontend-${LLVM_VERSION_MAJOR} PRIVATE ${MULL_CXX_LLVM_LIBRARIES} mull-configuration mull-diagnostics)

#INSTALL(TARGETS mull-cxx-frontend
#  RUNTIME DESTINATION lib
#)

if (APPLE)
  target_link_libraries(mull-cxx-frontend-${LLVM_VERSION_MAJOR} PRIVATE
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
  target_link_libraries(mull-cxx-frontend-${LLVM_VERSION_MAJOR} PRIVATE
    clang
  )
endif()
