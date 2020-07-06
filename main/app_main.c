/*
 * MIT License
 *
 * Copyright (c) 2017 David Antliff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "mqtt_client.h"

#include "main.h"
#include "wifi.h"
#include "mqtt.h"
#include "ltft.h"
#include "lowb.h"

static const char *TAG = "MQTT_APP";
static esp_mqtt_client_handle_t mqttClient;

static esp_mqtt_client_handle_t mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .username = CONFIG_MQTT_USERNAME,
        .password = CONFIG_MQTT_PASSWORD};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    return client;
}

void read_sensor_task(void *pvParameters) {
    char tmp_buff[64];
    if (xSensorQ != 0)
    {
        struct SMsg *pxSMsg;
        while(1) {
            if (xQueueReceive(xSensorQ, &(pxSMsg), (TickType_t)10) == pdTRUE)
            {
                // display sensor valus on LCD
                sprintf(tmp_buff, "Sensor - %d, T: %.1f C, Time: %d", pxSMsg->sensorId, pxSMsg->temp, (int)pxSMsg->time);
                TFT_print(tmp_buff, 10, 10 + (int)pxSMsg->sensorId * 15 + TFT_getfontheight());

                // send sensor value by MQTT
                if (mqttClient) {
                    char topic[64];
                    sprintf(topic, "%s/sensor/%d", CONFIG_MQTT_COMMON_TOPIC, pxSMsg->sensorId);
                    
                    char data[10];
                    sprintf(data, "%f", pxSMsg->temp);
                    esp_mqtt_client_publish(mqttClient, topic, data, 0, 1, 0);
                    printf("TOPIC: %s DATA: %s\r\n", topic, data);
                }
            }
        }
    }
}

_Noreturn void app_main()
{

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Override global log level
    esp_log_level_set("*", ESP_LOG_INFO);

    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (wifi_init_sta() == ESP_OK)
    {
        mqttClient = mqtt_app_start();
    }
    else
    {
        ESP_LOGI(TAG, "mqtt app failed, not starting wifi_init_sta service");
    }

    // Stable readings require a brief period before communication
    vTaskDelay(2000.0 / portTICK_PERIOD_MS);

    // --- display ----
    tft_init();
    TFT_setRotation(LANDSCAPE);
    TFT_setFont(DEFAULT_FONT, NULL);
    tft_fg = TFT_ORANGE;
    TFT_print("HeatingController", 10, 10);
    // -- end display --

    xSensorQ = xQueueCreate(10, sizeof(struct SMsg *));
    xTaskCreate(&read_sensor_task, "read_sensor_task", 2048, NULL, 5, NULL);

    owb_init(xSensorQ);
    

    ESP_LOGI(TAG, "App started");
    
    fflush(stdout);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
}
