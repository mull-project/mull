add_subdirectory(json11 EXCLUDE_FROM_ALL)
add_subdirectory(LibEBC EXCLUDE_FROM_ALL)

add_subdirectory(cpputest EXCLUDE_FROM_ALL)
target_compile_definitions(CppUTest
    PRIVATE CPPUTEST_MEM_LEAK_DETECTION_DISABLED
)
