/**
 * \copyright Copyright (c) 2021-2024, Buildstorm Pvt Ltd
 *
 * \file app_main.c
 * \brief app_main.c file.
 *
 * The app_main.c is the main entry of the application.
 *
 * The file have been tested on the ESP32 modules.
 * Buildstorm explicitly denies responsibility for any hardware failures
 * arising from the use of these file, whether directly or indirectly.
 * Please note that files are subject to change without prior notice.
 *
 * EULA LICENSE:
 * This file is licensed under end user license EULA agreement.
 * The EULA is available at https://buildstorm.com/eula/
 * For any support contact us at hello@buildstorm.com
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lib_system.h"
#include "lib_print.h"
#include "lib_delay.h"
#include "app_config.h"
#include "lib_ble.h"
#include "lib_attributes.h"
#include "lib_telemetry.h"

#define thisModule APP_MODULE_MAIN

void app_eventsCallBackHandler(systemEvents_et event_e)
{
    switch (event_e)
    {
    case EVENT_WIFI_CONNECTED:
        print_verbose("EVENT_WIFI_CONNECTED");
        break;
    case EVENT_WIFI_DISCONNECTED:
        print_verbose("EVENT_WIFI_DISCONNECTED");
        break;

    case EVENT_MQTT_CONNECTED:
        print_verbose("EVENT_MQTT_CONNECTED");
        break;
    case EVENT_MQTT_DISCONNECTED:
        print_verbose("EVENT_MQTT_DISCONNECTED");
        break;

    default:
        break;
    }
}

void app_task(void *param)
{
    mqttMsg_st s_mqttMsg = {0};
    static uint32_t nextPubTime = 0;
    uint32_t temp_u32 = 20;
    float humidity = 10.0;

    while (1)
    {
        if (SYSTEM_getMode() == SYSTEM_MODE_NORMAL)
        {
            if (TB_isConnected())
            {
                if ((millis() > nextPubTime))
                {
                    TELEMETRY_update("temp", &temp_u32, VALUE_TYPE_INT);
                    TELEMETRY_update("hum", &humidity, VALUE_TYPE_FLOAT);
                    print_verbose("[temp]:%d [hum]: %.2f\n", temp_u32, humidity);

                    temp_u32++;
                    humidity = humidity + 1.1;
                    nextPubTime = millis() + 5000;
                }
            }
        }

        TASK_DELAY_MS(200);
    }
}

/**
 * @brief    entry point of the project
 * @param    None
 * @return   None
 */
void app_main()
{

    systemInitConfig_st sysConfig = {
        .systemEventCallbackHandler = app_eventsCallBackHandler,
        .pAppVersionStr = APP_VERSION,
        .pWifiSsidStr = TEST_WIFI_SSID,
        .pWifiPwdStr = TEST_WIFI_PASSWORD,
        .maxTelemetryElements_u8 = 2,
        .s_mqttClientConfig = {
            .pUriStr = THINGSBOARD_URI,
            .pProvisionDeviceKey = PROVISION_DEVICE_KEY,
            .pProvisionDeviceSecret = PROVISION_DEVICE_SECRET}};

    if (SYSTEM_init(&sysConfig) == TRUE)
    {
        SYSTEM_start();

        BaseType_t err = xTaskCreate(&app_task, "app_task", TASK_APP_STACK_SIZE, NULL, TASK_APP_PRIORITY, NULL);
        if (pdPASS != err)
        {
            print_error("\n Error 0x%X in creating app_task \n restarting system\n\r\n\r", err);
            fflush(stdout);
            esp_restart();
        }
    }
    else
    {
        while (1)
        {
            print_verbose("System Init failed, verify the init config ....");
            TASK_DELAY_MS(5000);
        }
    }
}
