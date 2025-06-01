cmake_minimum_required(VERSION 3.0)

project (w25q128 VERSION 1.0.0)

set(SOURCES 
    w25q128.c    
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    stm32cubemx
)