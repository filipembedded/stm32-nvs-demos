project(littlefs VERSION 1.0.0)

set(SOURCES 
    ../../../../libraries/littlefs/lfs.c
    ../../../../libraries/littlefs/lfs_util.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ../../../../libraries/littlefs/
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
    w25q128_lfs
)