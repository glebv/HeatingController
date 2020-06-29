#include <ltft.h>

void tft_init() {
    esp_err_t ret;

    tft_max_rdclock = 8000000;

    TFT_PinsInit();

    spi_lobo_device_handle_t spi;

    spi_lobo_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO, // set SPI MISO pin
        .mosi_io_num = PIN_NUM_MOSI, // set SPI MOSI pin
        .sclk_io_num = PIN_NUM_CLK,  // set SPI CLK pin
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 6 * 1024,
    };
    spi_lobo_device_interface_config_t devcfg = {
        .clock_speed_hz = 8000000,         // Initial clock out at 8 MHz
        .mode = 0,                         // SPI mode 0
        .spics_io_num = -1,                // we will use external CS pin
        .spics_ext_io_num = PIN_NUM_CS,    // external CS pin
        .flags = LB_SPI_DEVICE_HALFDUPLEX, // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    vTaskDelay(500 / portTICK_RATE_MS);

    ret = spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
    assert(ret == ESP_OK);
    printf("SPI: display device added to spi bus (%d)\r\n", SPI_BUS);
    tft_disp_spi = spi;

    // ==== Test select/deselect ====
    ret = spi_lobo_device_select(spi, 1);
    assert(ret == ESP_OK);
    ret = spi_lobo_device_deselect(spi);
    assert(ret == ESP_OK);

    printf("SPI: attached display device, speed=%u\r\n", spi_lobo_get_speed(spi));
    printf("SPI: bus uses native pins: %s\r\n", spi_lobo_uses_native_pins(spi) ? "true" : "false");

    printf("SPI: display init...\r\n");
    TFT_display_init();
#ifdef TFT_START_COLORS_INVERTED
    TFT_invertDisplay(1);
#endif

    // ---- Detect maximum read speed ----
    tft_max_rdclock = find_rd_speed();
    printf("SPI: Max rd speed = %u\r\n", tft_max_rdclock);

    // ==== Set SPI clock used for display operations ====
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
    printf("SPI: Changed speed to %u\r\n", spi_lobo_get_speed(spi));

    printf("\r\n---------------------\r\n");
    printf("Graphics demo started\r\n");
    printf("---------------------\r\n");

    tft_font_rotate = 0;
    tft_text_wrap = 0;
    tft_font_transparent = 0;
    tft_font_forceFixed = 0;
    tft_gray_scale = 0;
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(PORTRAIT);
    TFT_setFont(DEFAULT_FONT, NULL);
    TFT_resetclipwin();

    // ==== Initialize the file system ====
    printf("\r\n\n");
    vfs_spiffs_register();
}