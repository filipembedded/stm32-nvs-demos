project (w25q128_lfs VERSION 1.0.0)

set(SOURCES 
    ${EXTERNAL_LIBS_DIR}/libraries/w25q128/littlefs-level-driver/w25q128_lfs.c    
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${EXTERNAL_LIBS_DIR}/libraries/w25q128/littlefs-level-driver/
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
    littlefs
    w25q128_ll
)
