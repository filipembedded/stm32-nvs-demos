add_library(legacy STATIC
    Src/legacy_fram.c
)

target_include_directories(legacy PUBLIC
    Inc/
)

target_link_libraries(legacy
    stm32cubemx
    littlefs
	w25q128_ll
    w25q128_lfs
)
