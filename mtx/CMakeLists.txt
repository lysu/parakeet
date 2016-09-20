set(SOURCE_FILES mtx.c)

include_directories(${PROJECT_SOURCE_DIR}/atomic
        ${PROJECT_SOURCE_DIR}/shm)

add_library(mtx ${SOURCE_FILES})