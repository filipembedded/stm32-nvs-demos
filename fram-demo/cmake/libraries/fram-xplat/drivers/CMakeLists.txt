project (fram_drivers VERSION 1.0.0)

set(SOURCES 
    ${EXTERNAL_LIBS_DIR}/libraries/fram-xplat/drivers/fram.c    
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${EXTERNAL_LIBS_DIR}/libraries/fram/drivers/
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
)
