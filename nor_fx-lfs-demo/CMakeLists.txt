cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME nor_fx-lfs-demo)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Explicitly select the NORFX littlefs adapter for this firmware build.
set(NORFX_BUILD_LFS ON CACHE BOOL "Build the NORFX littlefs adapter" FORCE)

# Explicitly select the NORFX STM32F4 port for this firmware build.
set(NORFX_BUILD_PORT_STM32_F4 ON CACHE BOOL "Build the NORFX STM32F4 port" FORCE)

# Resolve external libraries directory
if(NOT DEFINED EXTERNAL_LIBS_DIR)
    get_filename_component(EXTERNAL_LIBS_DIR "${CMAKE_SOURCE_DIR}/../libraries" ABSOLUTE)
endif()

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add libraries subdirectory
add_subdirectory(cmake/libraries)

# Add NORFX subdirectory
add_subdirectory(${EXTERNAL_LIBS_DIR}/nor_fx ${CMAKE_BINARY_DIR}/nor_fx)

# Provide HAL headers to the STM32 port
target_link_libraries(norfx_port_stm32 PRIVATE stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
    norfx_lfs
    norfx_port_stm32_f4
)
