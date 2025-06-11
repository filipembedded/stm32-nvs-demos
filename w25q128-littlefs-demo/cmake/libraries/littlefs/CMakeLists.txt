project(littlefs VERSION 1.0.0)

set(SOURCES 
    ${EXTERNAL_LIBS_DIR}/libraries/littlefs/lfs.c
    ${EXTERNAL_LIBS_DIR}/libraries/littlefs/lfs_util.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${EXTERNAL_LIBS_DIR}/libraries/littlefs/
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
    w25q128_lfs
)