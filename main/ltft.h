#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tftspi.h"
#include "tft.h"
#include "spiffs_vfs.h"

#define SPI_BUS TFT_HSPI_HOST

void tft_init();