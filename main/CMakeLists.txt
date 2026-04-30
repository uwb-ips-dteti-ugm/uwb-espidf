file(GLOB_RECURSE srcsc "src/*.c")
idf_component_register(
    SRCS 
        "main.c"
        ${srcsc}
    INCLUDE_DIRS
        "include"
    PRIV_REQUIRES 
        # built-in
        freertos
        spi_flash
        nvs_flash
        driver
        esp_timer
        esp_wifi
        esp_event
        esp_netif
        esp_https_ota
        app_update
        mbedtls
        json

        # idf components
        esp_websocket_client

        # project components
        dw3000
)
