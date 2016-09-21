include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(parakeet_test main.c)
target_link_libraries(parakeet_test limiter pthread)
