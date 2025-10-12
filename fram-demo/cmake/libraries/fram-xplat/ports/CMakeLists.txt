project (fram_port_stm32 VERSION 1.0.0)

set(SOURCES 
    ${EXTERNAL_LIBS_DIR}/libraries/fram-xplat/ports/stm32/port_stm32.c    
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${EXTERNAL_LIBS_DIR}/libraries/fram/ports/stm32
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
)
