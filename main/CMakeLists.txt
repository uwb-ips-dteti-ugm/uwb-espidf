file(GLOB_RECURSE srcsc "src/*.c")
idf_component_register(
    SRCS 
        "main.c"
        ${srcsc}
    INCLUDE_DIRS
        "include"
    PRIV_REQUIRES 
        spi_flash
)
