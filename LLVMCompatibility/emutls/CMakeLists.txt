add_library(mull-tls SHARED emutls.c)
target_include_directories(mull-tls SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
