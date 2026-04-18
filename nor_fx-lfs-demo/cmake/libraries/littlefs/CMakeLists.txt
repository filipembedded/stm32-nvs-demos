project(lfs VERSION 1.0.0)

set(SOURCES
    ${EXTERNAL_LIBS_DIR}/littlefs/lfs.c
    ${EXTERNAL_LIBS_DIR}/littlefs/lfs_util.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(littlefs ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${EXTERNAL_LIBS_DIR}/littlefs
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
)