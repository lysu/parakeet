include_directories(${PROJECT_SOURCE_DIR}/include
        ${CUNIT_INCLUDE_DIRS})
find_package(CUnit 2.1)
add_executable(parakeet_test suit.c)
enable_testing()
add_test(parakeet_test1 parakeet_test)
target_link_libraries(parakeet_test limiter pthread ${CUNIT_LIBRARIES})
