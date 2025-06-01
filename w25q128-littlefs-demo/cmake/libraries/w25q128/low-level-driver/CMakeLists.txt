project (w25q128_ll VERSION 1.0.0)

set(SOURCES 
    ../../../../../libraries/w25q128/low-level-driver/w25q128_ll.c    
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../../../../libraries/w25q128/low-level-driver/
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
)