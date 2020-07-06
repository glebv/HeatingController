#ifndef __LOWB_H__
#define __LOWB_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "main.h"

#define GPIO_DS18B20_0 (CONFIG_ONE_WIRE_GPIO)
#define MAX_DEVICES (8)
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD (1000) // milliseconds

void owb_init(QueueHandle_t xSensorQ);

#endif /* __LOWB_H__ */