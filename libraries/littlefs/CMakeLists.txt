project(littlefs VERSION 1.0.0)

set(SOURCES 
    lfs.c
    lfs_util.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
    nor_flash
)