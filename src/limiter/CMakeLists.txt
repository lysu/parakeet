add_library(limiter STATIC limiter.c)
include_directories(${PROJECT_SOURCE_DIR}/include)
target_link_libraries(limiter mtx shm)
