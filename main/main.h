#ifndef __MAIN_H__
#define __MAIN_H__

#include "freertos/queue.h"

struct SMsg // sensor message
{
    char sensorId;
    float temp;
    int64_t time;
};

QueueHandle_t xSensorQ; // sensor queue

#endif /* __MAIN_H__ */