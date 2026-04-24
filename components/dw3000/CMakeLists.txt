file(GLOB_RECURSE srcsc "src/*.c")
idf_component_register(
    SRCS 
        ${srcsc}
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        esp_driver_spi
)
