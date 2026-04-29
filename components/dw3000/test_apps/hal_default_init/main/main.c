#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "driver/spi_master.h"
#include "dw3000_error.h"
#include "dw3000_espidf.h"
#include "dw3000_hal/aes.h"
#include "dw3000_hal/cia.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/gpio.h"
#include "dw3000_hal/mac.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_hal/status.h"
#include "dw3000_hal/sts.h"
#include "dw3000_register.h"
#include "esp_err.h"
#include "esp_log.h"

static const char* TAG = "dw3000_hal_default_init";

typedef struct {
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
} dw3000_hal_default_init_app_t;

static bool dw3000_hal_default_init_check_esp(
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

static bool dw3000_hal_default_init_check_dw3000(
    dw3000_error_t err,
    const char*    expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

static bool dw3000_hal_default_init_check_u64(
    const char* name,
    uint64_t    actual,
    uint64_t    expected
) {
    if (actual == expected) {
        return true;
    }

    ESP_LOGE(
        TAG,
        "%s mismatch: actual=0x%016" PRIX64 " expected=0x%016" PRIX64,
        name,
        actual,
        expected
    );
    return false;
}

#define DW3000_HAL_DEFAULT_INIT_CHECK_ESP(expr_) \
    dw3000_hal_default_init_check_esp((expr_), #expr_)

#define DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(expr_) \
    dw3000_hal_default_init_check_dw3000((expr_), #expr_)

static bool dw3000_hal_default_init_setup_spi(
    dw3000_hal_default_init_app_t* app
) {
    spi_bus_config_t bus_config = {
        .mosi_io_num     = DW3000_HAL_DEFAULT_INIT_PIN_MOSI,
        .miso_io_num     = DW3000_HAL_DEFAULT_INIT_PIN_MISO,
        .sclk_io_num     = DW3000_HAL_DEFAULT_INIT_PIN_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = DW3000_HAL_DEFAULT_INIT_SPI_MAX_TRANSFER_SIZE,
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = DW3000_HAL_DEFAULT_INIT_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = DW3000_HAL_DEFAULT_INIT_PIN_CS,
        .queue_size     = 1,
    };
    esp_err_t err;

    err = spi_bus_initialize(
        DW3000_HAL_DEFAULT_INIT_SPI_HOST,
        &bus_config,
        SPI_DMA_CH_AUTO
    );
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return DW3000_HAL_DEFAULT_INIT_CHECK_ESP(err);
    }

    return DW3000_HAL_DEFAULT_INIT_CHECK_ESP(spi_bus_add_device(
        DW3000_HAL_DEFAULT_INIT_SPI_HOST,
        &device_config,
        &app->spi
    ));
}

static bool dw3000_hal_default_init_setup_port(
    dw3000_hal_default_init_app_t* app
) {
    dw3000_espidf_port_config_t port_config;

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(TAG, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = DW3000_HAL_DEFAULT_INIT_PIN_RST;
    port_config.irq_gpio             = DW3000_HAL_DEFAULT_INIT_PIN_IRQ;
    port_config.configure_reset_gpio = DW3000_HAL_DEFAULT_INIT_CONFIGURE_RST;
    port_config.configure_irq_gpio   = DW3000_HAL_DEFAULT_INIT_CONFIGURE_IRQ;

    return DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_espidf_port_configure(
        app->port,
        &port_config
    ));
}

static bool dw3000_hal_default_init_setup(
    dw3000_hal_default_init_app_t* app
) {
    memset(app, 0, sizeof(*app));

    if (!dw3000_hal_default_init_setup_spi(app)) {
        return false;
    }

    if (!dw3000_hal_default_init_setup_port(app)) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        DW3000_HAL_DEFAULT_INIT_SPI_HOST,
        DW3000_HAL_DEFAULT_INIT_SPI_CLOCK_HZ,
        DW3000_HAL_DEFAULT_INIT_PIN_SCK,
        DW3000_HAL_DEFAULT_INIT_PIN_MISO,
        DW3000_HAL_DEFAULT_INIT_PIN_MOSI,
        DW3000_HAL_DEFAULT_INIT_PIN_CS,
        DW3000_HAL_DEFAULT_INIT_PIN_RST,
        DW3000_HAL_DEFAULT_INIT_PIN_IRQ
    );

    return true;
}

static bool dw3000_hal_default_init_teardown(
    dw3000_hal_default_init_app_t* app
) {
    bool ok = true;

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok       = DW3000_HAL_DEFAULT_INIT_CHECK_ESP(spi_bus_remove_device(app->spi)) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok = DW3000_HAL_DEFAULT_INIT_CHECK_ESP(spi_bus_free(
            DW3000_HAL_DEFAULT_INIT_SPI_HOST
        )) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

static bool dw3000_hal_default_init_run_initialize(
    dw3000_hal_default_init_app_t* app,
    dw3000_device_config_t*        expected_config
) {
    dw3000_hal_init_options_t options;

    dw3000_hal_default_config(expected_config);
    expected_config->load_otp_calibration =
        DW3000_HAL_DEFAULT_INIT_LOAD_OTP_CALIBRATION;
    expected_config->auto_init_pll =
        DW3000_HAL_DEFAULT_INIT_ENTER_IDLE_PLL;

    dw3000_hal_default_init_options(&options);

    return DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_initialize(
        &app->device,
        NULL,
        &options,
        app->port,
        expected_config
    ));
}

static bool dw3000_hal_default_init_check_state(
    const dw3000_device_t* device
) {
    dw3000_device_state_flags_t expected = (dw3000_device_state_flags_t)(
        DW3000_DEVICE_STATE_PORT_READY |
        DW3000_DEVICE_STATE_PRESENT |
        DW3000_DEVICE_STATE_INITIALIZED
    );

    if (DW3000_HAL_DEFAULT_INIT_ENTER_IDLE_PLL) {
        expected = (dw3000_device_state_flags_t)(
            expected | DW3000_DEVICE_STATE_IDLE_PLL
        );
    }

    ESP_LOGI(TAG, "state_flags=0x%08" PRIX32, (uint32_t)device->state_flags);
    return dw3000_hal_default_init_check_u64(
        "required state flags",
        (uint64_t)(device->state_flags & expected),
        (uint64_t)expected
    );
}

static bool dw3000_hal_default_init_check_mac(
    dw3000_device_t*              device,
    const dw3000_device_config_t* expected
) {
    dw3000_mac_eui_t    eui;
    dw3000_mac_panadr_t panadr;
    bool                ok = true;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_mac_read_eui(
            device,
            &eui
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_mac_read_panadr(
            device,
            &panadr
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "EUI=0x%016" PRIX64, eui);
    ESP_LOGI(
        TAG,
        "PANADR pan_id=0x%04" PRIX16 " short_addr=0x%04" PRIX16,
        panadr.pan_id,
        panadr.short_addr
    );

    ok = dw3000_hal_default_init_check_u64(
        "EUI",
        eui,
        expected->mac.eui
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "PAN ID",
        panadr.pan_id,
        expected->mac.panadr.pan_id
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "short address",
        panadr.short_addr,
        expected->mac.panadr.short_addr
    ) && ok;

    return ok;
}

static bool dw3000_hal_default_init_check_sts(
    dw3000_device_t*              device,
    const dw3000_device_config_t* expected
) {
    dw3000_sts_config_t sts;
    bool                ok = true;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_sts_read_config(
            device,
            &sts
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "STS cps_len=%u packet_cfg=%u pdoa=%u sys_cfg_flags=0x%08" PRIX32,
        sts.cps_len,
        (unsigned)sts.packet_cfg,
        (unsigned)sts.pdoa_mode,
        (uint32_t)sts.sys_cfg_flags
    );

    ok = dw3000_hal_default_init_check_u64(
        "STS CPS length",
        sts.cps_len,
        expected->sts.cps_len
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "STS packet config",
        sts.packet_cfg,
        expected->sts.packet_cfg
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "STS PDoA mode",
        sts.pdoa_mode,
        expected->sts.pdoa_mode
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "STS SYS_CFG flags",
        sts.sys_cfg_flags,
        expected->sts.sys_cfg_flags
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "STS IV byte 0",
        sts.iv.bytes[0],
        expected->sts.iv.bytes[0]
    ) && ok;

    return ok;
}

static bool dw3000_hal_default_init_read_cia(dw3000_device_t* device) {
    dw3000_cia_config_t cia;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_cia_read_config(
            device,
            &cia
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "CIA rx_antd=0x%04" PRIX16 " tx_antd=0x%04" PRIX16
        " conf=0x%08" PRIX32 " adjust=0x%04" PRIX16,
        cia.rx_antd,
        cia.tx_antd,
        (uint32_t)cia.conf_flags,
        cia.adjust
    );
    ESP_LOGI(
        TAG,
        "CIA fp_agreed_th=%u tc_rcfg=%u ip_ntm=%u ip_pmult=%u ip_rtm=%u",
        cia.fp_conf.fp_agreed_th,
        cia.fp_conf.tc_rcfg,
        cia.ip_conf.ntm,
        cia.ip_conf.pmult,
        cia.ip_conf.rtm
    );
    ESP_LOGI(
        TAG,
        "CIA sts_ntm=%u sts_pmult=%u sts_rtm=%u sts_mnth=%u sts_cq_en=%u",
        cia.sts_conf.ntm,
        cia.sts_conf.pmult,
        cia.sts_conf.rtm,
        cia.sts_conf.mnth,
        cia.sts_conf.cq_en ? 1U : 0U
    );

    return true;
}

static bool dw3000_hal_default_init_check_gpio(
    dw3000_device_t*              device,
    const dw3000_device_config_t* expected
) {
    dw3000_gpio_mode_t mode;
    dw3000_gpio_pin_t  pull_enable;
    dw3000_gpio_pin_t  input;
    dw3000_gpio_pin_t  output;
    dw3000_gpio_raw_t  raw;
    bool               ok = true;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_gpio_read_mode(
            device,
            &mode
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_gpio_read_pull_enable(
            device,
            &pull_enable
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_gpio_read_direction(
            device,
            &input
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_gpio_read_output(
            device,
            &output
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_gpio_read_raw(
            device,
            &raw
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "GPIO pull=0x%04" PRIX16 " input=0x%04" PRIX16
        " output=0x%04" PRIX16 " raw=0x%04" PRIX16,
        pull_enable,
        input,
        output,
        raw
    );
    ESP_LOGI(
        TAG,
        "GPIO mode msgp0=%u msgp1=%u msgp2=%u msgp3=%u msgp4=%u msgp5=%u msgp6=%u msgp7=%u msgp8=%u",
        mode.msgp[0],
        mode.msgp[1],
        mode.msgp[2],
        mode.msgp[3],
        mode.msgp[4],
        mode.msgp[5],
        mode.msgp[6],
        mode.msgp[7],
        mode.msgp[8]
    );

    ok = dw3000_hal_default_init_check_u64(
        "GPIO pull enable",
        pull_enable,
        expected->gpio.pull_enable
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "GPIO input direction",
        input,
        expected->gpio.input
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "GPIO output",
        output,
        expected->gpio.output
    ) && ok;

    for (uint8_t i = 0U; i < DW3000_GPIO_PIN_COUNT; ++i) {
        ok = dw3000_hal_default_init_check_u64(
            "GPIO mode",
            mode.msgp[i],
            expected->gpio.mode.msgp[i]
        ) && ok;
    }

    return ok;
}

static bool dw3000_hal_default_init_check_aes(
    dw3000_device_t*              device,
    const dw3000_device_config_t* expected
) {
    dw3000_aes_config_t aes;
    dw3000_aes_dma_cfg_t dma;
    dw3000_aes_status_t status;
    bool                ok = true;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_aes_read_cfg(
            device,
            &aes
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_aes_read_dma_cfg(
            device,
            &dma
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_aes_read_status(
            device,
            &status
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "AES mode=%u key_size=%u tag_size=%u core=%u key_src=%u key_addr=%u flags=0x%04" PRIX16,
        (unsigned)aes.mode,
        (unsigned)aes.key_size,
        (unsigned)aes.tag_size,
        (unsigned)aes.core,
        (unsigned)aes.key_src,
        aes.key_addr,
        (uint16_t)aes.flags
    );
    ESP_LOGI(
        TAG,
        "AES DMA src=%u:%u dst=%u:%u end=%u hdr=%u pyld=%u status=0x%02" PRIX32,
        (unsigned)dma.src_port,
        dma.src_addr,
        (unsigned)dma.dst_port,
        dma.dst_addr,
        (unsigned)dma.endianness,
        dma.hdr_size,
        dma.pyld_size,
        (uint32_t)status
    );

    ok = dw3000_hal_default_init_check_u64(
        "AES mode",
        aes.mode,
        expected->aes.mode
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "AES key size",
        aes.key_size,
        expected->aes.key_size
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "AES tag size",
        aes.tag_size,
        expected->aes.tag_size
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "AES core",
        aes.core,
        expected->aes.core
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "AES key source",
        aes.key_src,
        expected->aes.key_src
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "AES DMA source port",
        dma.src_port,
        expected->aes.dma.src_port
    ) && ok;
    ok = dw3000_hal_default_init_check_u64(
        "AES DMA destination port",
        dma.dst_port,
        expected->aes.dma.dst_port
    ) && ok;

    return ok;
}

static bool dw3000_hal_default_init_read_pmsc(dw3000_device_t* device) {
    dw3000_pmsc_clk_ctrl_t    clk;
    dw3000_pmsc_seq_ctrl_flags_t seq;
    dw3000_pmsc_txfseq_t      txfseq;
    dw3000_pmsc_led_ctrl_t    led;
    dw3000_pmsc_bias_ctrl_t   bias;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_pmsc_read_clock_ctrl(
            device,
            &clk
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_pmsc_read_seq_ctrl(
            device,
            &seq
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_pmsc_read_txfseq(
            device,
            &txfseq
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_pmsc_read_led_ctrl(
            device,
            &led
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_pmsc_read_bias_ctrl(
            device,
            &bias
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "PMSC clk sys=%u rx=%u tx=%u flags=0x%08" PRIX32
        " seq=0x%08" PRIX32,
        (unsigned)clk.sys_clk,
        (unsigned)clk.rx_clk,
        (unsigned)clk.tx_clk,
        (uint32_t)clk.flags,
        (uint32_t)seq
    );
    ESP_LOGI(
        TAG,
        "PMSC txfseq=0x%08" PRIX32 " led_blink=%u led_flags=0x%08" PRIX32
        " bias=0x%04" PRIX16,
        txfseq,
        led.blink_tim,
        (uint32_t)led.flags,
        bias
    );

    return true;
}

static bool dw3000_hal_default_init_read_raw_registers(
    dw3000_device_t* device
) {
    dw3000_txrx_event_t status;
    dw3000_txrx_event_t enabled_events;
    uint32_t            sys_cfg;
    uint32_t            tx_fctrl;
    uint32_t            sys_state;
    uint32_t            dtune3;
    uint16_t            chan_ctrl;
    uint16_t            dtune0;
    uint16_t            dgc_cfg;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_status_read(
            device,
            &status
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_status_read_enabled(
            device,
            &enabled_events
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_SYS_CFG,
            &sys_cfg
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_TX_FCTRL,
            &tx_fctrl
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u16(
            device,
            DW3000_REG_CHAN_CTRL,
            &chan_ctrl
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u16(
            device,
            DW3000_REG_DTUNE0,
            &dtune0
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_DTUNE3,
            &dtune3
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u16(
            device,
            DW3000_REG_DGC_CFG,
            &dgc_cfg
        )) ||
        !DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_SYS_STATE,
            &sys_state
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "SYS_STATUS=0x%012" PRIX64, (uint64_t)status);
    ESP_LOGI(TAG, "SYS_ENABLE=0x%012" PRIX64, (uint64_t)enabled_events);
    ESP_LOGI(TAG, "SYS_CFG=0x%08" PRIX32, sys_cfg);
    ESP_LOGI(TAG, "TX_FCTRL=0x%08" PRIX32, tx_fctrl);
    ESP_LOGI(TAG, "CHAN_CTRL=0x%04" PRIX16, chan_ctrl);
    ESP_LOGI(TAG, "DTUNE0=0x%04" PRIX16, dtune0);
    ESP_LOGI(TAG, "DTUNE3=0x%08" PRIX32, dtune3);
    ESP_LOGI(TAG, "DGC_CFG=0x%04" PRIX16, dgc_cfg);
    ESP_LOGI(TAG, "SYS_STATE=0x%08" PRIX32, sys_state);

    return true;
}

static bool dw3000_hal_default_init_readback(
    dw3000_hal_default_init_app_t* app,
    const dw3000_device_config_t*  expected
) {
    dw3000_device_id_t id;
    bool               ok = true;

    if (!DW3000_HAL_DEFAULT_INIT_CHECK_DW3000(dw3000_hal_read_device_id(
            &app->device,
            &id
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "DEV_ID ridtag=0x%04" PRIX16 " model=0x%02" PRIX8
        " ver=0x%01" PRIX8 " rev=0x%01" PRIX8,
        id.ridtag,
        id.model,
        id.ver,
        id.rev
    );

    if (!dw3000_hal_is_supported_device_id(&id)) {
        ESP_LOGE(TAG, "Unsupported DEV_ID");
        return false;
    }

    ok = dw3000_hal_default_init_check_state(&app->device) && ok;
    ok = dw3000_hal_default_init_check_mac(&app->device, expected) && ok;
    ok = dw3000_hal_default_init_check_sts(&app->device, expected) && ok;
    ok = dw3000_hal_default_init_read_cia(&app->device) && ok;
    ok = dw3000_hal_default_init_check_gpio(&app->device, expected) && ok;
    ok = dw3000_hal_default_init_check_aes(&app->device, expected) && ok;
    ok = dw3000_hal_default_init_read_pmsc(&app->device) && ok;
    ok = dw3000_hal_default_init_read_raw_registers(&app->device) && ok;

    return ok;
}

void app_main(void) {
    dw3000_hal_default_init_app_t app;
    dw3000_device_config_t        expected_config;
    bool                          ok;

    ok = dw3000_hal_default_init_setup(&app) &&
         dw3000_hal_default_init_run_initialize(&app, &expected_config) &&
         dw3000_hal_default_init_readback(&app, &expected_config);

    ok = dw3000_hal_default_init_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_DEFAULT_INIT_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_DEFAULT_INIT_FAIL");
    }
}
