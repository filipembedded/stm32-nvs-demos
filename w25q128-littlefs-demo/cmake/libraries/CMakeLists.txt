if (NOT DEFINED EXTERNAL_LIBS_DIR)
    message(INFO ": EXTERNAL_LIBS_DIR not explicitly specified, \
setting parent directory of the demo as a library path.")
    get_filename_component(EXTERNAL_LIBS_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
endif()

add_subdirectory(littlefs)
add_subdirectory(w25q128/littlefs-level-driver)
add_subdirectory(w25q128/low-level-driver)