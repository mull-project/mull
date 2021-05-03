add_library(mull-cxx-frontend
  SHARED
  src/ASTInstrumentation.cpp
  src/ASTMutationsSearchVisitor.cpp
  src/ASTMutation.cpp
  src/ASTNodeFactory.cpp
  src/MullClangPlugin.cpp

  ${CMAKE_SOURCE_DIR}/lib/AST/ASTConstants.cpp
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
  LLVMAnalysis
  LLVMScalarOpts
  LLVMCoroutines
  LLVMAggressiveInstCombine
  LLVMAsmPrinter
  LLVMAsmParser
  LLVMBinaryFormat
  LLVMBitstreamReader
  LLVMDebugInfoDWARF
  LLVMInstCombine
  LLVMMCDisassembler
  LLVMSupport
  LLVMCodeGen
  LLVMCore
  LLVMCoverage
  LLVMBitReader
  LLVMBitWriter
  LLVMDebugInfoCodeView
  LLVMDemangle
  LLVMGlobalISel
  LLVMInstrumentation
  LLVMIRReader
  LLVMLinker
  LLVMLTO
  LLVMMC
  LLVMMCParser
  LLVMPasses
  LLVMProfileData
  LLVMObject
  LLVMOption
  LLVMObjCARCOpts
  LLVMRemarks
  LLVMSelectionDAG
  LLVMSupport
  LLVMTarget
  LLVMTransformUtils
  LLVMX86AsmParser
  LLVMX86CodeGen
  LLVMX86Info
  LLVMX86Desc
  LLVMX86Utils
  LLVMVectorize
  LLVMipo
  LTO
  ncurses
  z
)
