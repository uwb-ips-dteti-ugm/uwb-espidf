file(GLOB_RECURSE srcsc "src/*.c")
idf_component_register(
    SRCS 
        ${srcsc}
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        esp_driver_gpio
        esp_driver_spi
    PRIV_REQUIRES
        esp_rom
        esp_timer
        freertos
)
