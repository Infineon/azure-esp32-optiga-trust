/**
 * MIT License
 *
 * Copyright (c) 2026 Infineon Technologies AG
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
 * SOFTWARE
 *
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "certs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#include "azure_prov_client/prov_security_factory.h"

#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "iothubtransportmqtt.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"

#include "custom_hsm_optiga.h"

#include "sdkconfig.h"


static const char *TAG = "dps_sample";
static const char DPS_ROOT_CA[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
"MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n"
"MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n"
"UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n"
"eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n"
"GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n"
"mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n"
"SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n"
"dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n"
"AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n"
"VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n"
"buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n"
"AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n"
"b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n"
"Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n"
"oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n"
"dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n"
"AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n"
"klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n"
"kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n"
"eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n"
"penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n"
"QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n"
"q/xKzj3O9hFh/g==\n"
"-----END CERTIFICATE-----\n";




// ─────────────────────────────────────────────────────────────────────────────
// DPS configuration (from Kconfig / menuconfig)
// ─────────────────────────────────────────────────────────────────────────────
#define DPS_GLOBAL_ENDPOINT  CONFIG_AZURE_DPS_ENDPOINT
#define DPS_ID_SCOPE         CONFIG_AZURE_DPS_ID_SCOPE
#define DPS_REGISTRATION_ID  CONFIG_AZURE_DPS_REGISTRATION_ID

// ─────────────────────────────────────────────────────────────────────────────
// Wi-Fi configuration (from Kconfig / menuconfig)
// ─────────────────────────────────────────────────────────────────────────────
#define WIFI_SSID            CONFIG_WIFI_SSID
#define WIFI_PASSWORD        CONFIG_WIFI_PASSWORD

// ─────────────────────────────────────────────────────────────────────────────
// Telemetry settings
// ─────────────────────────────────────────────────────────────────────────────
#define TELEMETRY_MSG_COUNT  5      // Number of messages to send after provisioning
#define SDK_DOWORK_DELAY_MS  100    // Polling interval for DoWork loops

// ─────────────────────────────────────────────────────────────────────────────
// Shared state between DPS registration callback and main task
// ─────────────────────────────────────────────────────────────────────────────
typedef struct {
    volatile bool  registration_complete;
    volatile bool  registration_ok;
    char           iothub_uri[256];
    char           device_id[128];
} DPS_STATE;

static DPS_STATE s_dps = {0};


static const char *prov_result_str(PROV_DEVICE_RESULT r) 
{
        switch(r) {
            case PROV_DEVICE_RESULT_OK:                 return "OK";
            case PROV_DEVICE_RESULT_INVALID_ARG:        return "INVALID ARG";
            case PROV_DEVICE_RESULT_SUCCESS:            return "SUCCESS";
            case PROV_DEVICE_RESULT_MEMORY:             return "MEMORY";
            case PROV_DEVICE_RESULT_PARSING:            return "PARSING";
            case PROV_DEVICE_RESULT_TRANSPORT:          return "TRANSPORT";
            case PROV_DEVICE_RESULT_INVALID_STATE:      return "INVALID_STATE";
            case PROV_DEVICE_RESULT_DEV_AUTH_ERROR:     return "DEV_AUTH_ERROR";
            case PROV_DEVICE_RESULT_TIMEOUT:            return "TIMEOUT";
            case PROV_DEVICE_RESULT_KEY_ERROR:          return "KEY_ERROR";
            case PROV_DEVICE_RESULT_ERROR:              return "ERROR";
            default:                                    return "UNKNOWN";
        }
}
// ─────────────────────────────────────────────────────────────────────────────
// Wi-Fi helpers
// ─────────────────────────────────────────────────────────────────────────────
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disc = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "Wi-Fi disconnected (reason %d), reconnecting...", disc->reason);
        esp_wifi_connect();

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t inst_wifi;
    esp_event_handler_instance_t inst_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &inst_wifi));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &inst_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for Wi-Fi...");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                        pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi connected.");
}

static void dps_status_callback(PROV_DEVICE_REG_STATUS status, void *user_context)
{
    (void)user_context;
    ESP_LOGI(TAG, "DPS status: %s (%d)", prov_result_str(status), (int)status);
}

static void dps_register_callback(PROV_DEVICE_RESULT register_result,
                                  const char *iothub_uri,
                                  const char *device_id,
                                  void *user_context)
{
    DPS_STATE *state = (DPS_STATE *)user_context;

    if (register_result == PROV_DEVICE_RESULT_OK) {
        ESP_LOGI(TAG, "DPS registration succeeded!");
        ESP_LOGI(TAG, "  Assigned Hub : %s", iothub_uri);
        ESP_LOGI(TAG, "  Device ID    : %s", device_id);
        strncpy(state->iothub_uri, iothub_uri, sizeof(state->iothub_uri) - 1);
        strncpy(state->device_id,  device_id,  sizeof(state->device_id)  - 1);
        state->registration_ok = true;
    } else {
        ESP_LOGE(TAG, "DPS registration FAILED: %s (%d)",
                 prov_result_str(register_result), (int)register_result);
        state->registration_ok = false;
    }
    state->registration_complete = true;
}

static bool run_dps_provisioning(void)
{
    bool success = false;

    ESP_LOGI(TAG, "Initialising DPS security (X.509 / custom HSM)...");
 
    if (prov_dev_security_init(SECURE_DEVICE_TYPE_X509) != 0) {
        ESP_LOGE(TAG, "prov_dev_security_init() failed.");
        return false;
    }

    if (platform_init() != 0) {
        ESP_LOGE(TAG, "platform_init() failed.");
        prov_dev_security_deinit();
        return false;
    }

    ESP_LOGI(TAG, "Creating DPS client handle...");
    PROV_DEVICE_LL_HANDLE prov_handle = Prov_Device_LL_Create(
            DPS_GLOBAL_ENDPOINT,
            DPS_ID_SCOPE,
            Prov_Device_MQTT_Protocol);

    if (prov_handle == NULL) {
        ESP_LOGE(TAG, "Prov_Device_LL_Create() failed.");
        goto cleanup_platform;
    }

    if (Prov_Device_LL_SetOption(prov_handle, OPTION_TRUSTED_CERT, DPS_ROOT_CA) != PROV_DEVICE_RESULT_OK) {
        ESP_LOGE(TAG, "Failed to set DPS trusted cert.");
        goto cleanup_handle;
    }
    // Enable SDK logging (set to true for verbose TLS/MQTT trace)
    bool log_trace = false;
    Prov_Device_LL_SetOption(prov_handle, PROV_OPTION_LOG_TRACE, &log_trace);

    // The SDK will call the custom HSM to get the certificate and common name,
    // perform the TLS handshake (with OPTIGA signing via mbedTLS alt hooks),
    // and call dps_register_callback when done.
    s_dps.registration_complete = false;
    s_dps.registration_ok       = false;

    ESP_LOGI(TAG, "Starting DPS registration (ID Scope: %s, Reg ID: %s)...",
             DPS_ID_SCOPE, DPS_REGISTRATION_ID);

    PROV_DEVICE_RESULT reg_result = Prov_Device_LL_Register_Device(
            prov_handle,
            dps_register_callback,
            &s_dps,
            dps_status_callback,
            &s_dps);

    if (reg_result != PROV_DEVICE_RESULT_OK) {
        ESP_LOGE(TAG, "Prov_Device_LL_Register_Device() failed: %s (%d)",
                 prov_result_str(reg_result), (int)reg_result);

        goto cleanup_handle;
    }

    while (!s_dps.registration_complete) {
        Prov_Device_LL_DoWork(prov_handle);
        ThreadAPI_Sleep(SDK_DOWORK_DELAY_MS);
    }

    success = s_dps.registration_ok;

cleanup_handle:
    Prov_Device_LL_Destroy(prov_handle);
cleanup_platform:
    platform_deinit();
    prov_dev_security_deinit();
    return success;
}

static void iothub_send_confirmation_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result,
                                              void *user_context)
{
    size_t msg_id = (size_t)user_context;
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        ESP_LOGI(TAG, "Message [%zu] confirmed OK.", msg_id);
    } else {
        ESP_LOGW(TAG, "Message [%zu] confirmation: %s.", msg_id,
                 MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    }
}

static IOTHUBMESSAGE_DISPOSITION_RESULT iothub_receive_callback(
        IOTHUB_MESSAGE_HANDLE message, void *user_context)
{
    (void)user_context;
    const unsigned char *data = NULL;
    size_t len = 0;
    if (IoTHubMessage_GetByteArray(message, &data, &len) == IOTHUB_MESSAGE_OK) {
        ESP_LOGI(TAG, "C2D received [%zu bytes]: %.*s", len, (int)len, (char *)data);
    }
    return IOTHUBMESSAGE_ACCEPTED;
}

static void iothub_connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result,
                                              IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                                              void *user_context)
{
    (void)user_context;
    ESP_LOGI(TAG, "IoT Hub connection: %s (%s)",
             MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS, result),
             MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
}

static void run_iothub_telemetry(void)
{
    ESP_LOGI(TAG, "Connecting to IoT Hub: %s  (device: %s)",
             s_dps.iothub_uri, s_dps.device_id);

    if (platform_init() != 0) {
        ESP_LOGE(TAG, "platform_init() failed for IoT Hub phase.");
        return;
    }

    // IoTHub_Init() sets up the SDK subsystem (safe to call multiple times).
    if (IoTHub_Init() != 0) {
        ESP_LOGE(TAG, "IoTHub_Init() failed.");
        platform_deinit();
        return;
    }

    // CreateFromDeviceAuth() uses the same SECURE_DEVICE_TYPE_X509 /
    // custom HSM that DPS registered with — no extra certificate options needed.
    IOTHUB_DEVICE_CLIENT_LL_HANDLE hub_handle =
        IoTHubDeviceClient_LL_CreateFromDeviceAuth(
            s_dps.iothub_uri,
            s_dps.device_id,
            MQTT_Protocol);

    if (hub_handle == NULL) {
        ESP_LOGE(TAG, "IoTHubDeviceClient_LL_CreateFromDeviceAuth() failed.");
        goto cleanup;
    }
    // CRITICAL: Set the trusted cert for the IoT Hub - same cert as DPS
    // Without this esp-tls refuses to connect
    if(IoTHubDeviceClient_LL_SetOption(hub_handle, OPTION_TRUSTED_CERT, DPS_ROOT_CA) != IOTHUB_CLIENT_OK) {
        ESP_LOGE(TAG, "Failed to set IoT Hub trusted cert.");
        goto cleanup;
    }
    // Optional: enable SDK-level MQTT/TLS logging
    bool log_trace = false;
    IoTHubDeviceClient_LL_SetOption(hub_handle, OPTION_LOG_TRACE, &log_trace);

    // Register callbacks
    IoTHubDeviceClient_LL_SetConnectionStatusCallback(
            hub_handle, iothub_connection_status_callback, NULL);
    IoTHubDeviceClient_LL_SetMessageCallback(
            hub_handle, iothub_receive_callback, NULL);

    // ── Send telemetry messages ───────────────────────────────────────────────
    char msg_buf[256];
    size_t confirmed = 0;

    for (size_t i = 0; i < TELEMETRY_MSG_COUNT; i++) {
        // Simple JSON payload — extend to real sensor data as needed
        snprintf(msg_buf, sizeof(msg_buf),
                 "{\"deviceId\":\"%s\",\"msgId\":%zu,\"temperature\":%.1f,\"humidity\":%.1f}",
                 s_dps.device_id, i,
                 20.0f + (float)(rand() % 10),
                 50.0f + (float)(rand() % 20));

        IOTHUB_MESSAGE_HANDLE msg_handle =
            IoTHubMessage_CreateFromByteArray(
                (const unsigned char *)msg_buf, strlen(msg_buf));

        if (msg_handle == NULL) {
            ESP_LOGE(TAG, "IoTHubMessage_CreateFromByteArray() failed (msg %zu).", i);
            continue;
        }

        // Tag the message as JSON
        IoTHubMessage_SetContentTypeSystemProperty(msg_handle, "application/json");
        IoTHubMessage_SetContentEncodingSystemProperty(msg_handle, "utf-8");

        IOTHUB_CLIENT_RESULT send_result =
            IoTHubDeviceClient_LL_SendEventAsync(
                hub_handle, msg_handle,
                iothub_send_confirmation_callback,
                (void *)i);          // pass msg index as user context

        if (send_result == IOTHUB_CLIENT_OK) {
            ESP_LOGI(TAG, "Queued msg [%zu]: %s", i, msg_buf);
            confirmed++;
        } else {
            ESP_LOGE(TAG, "SendEventAsync failed (msg %zu): %s", i,
                     MU_ENUM_TO_STRING(IOTHUB_CLIENT_RESULT, send_result));
            IoTHubMessage_Destroy(msg_handle);
        }

        // Pump the SDK between sends
        for (int j = 0; j < 5; j++) {
            IoTHubDeviceClient_LL_DoWork(hub_handle);
            ThreadAPI_Sleep(SDK_DOWORK_DELAY_MS);
        }
    }

    ESP_LOGI(TAG, "Waiting for send confirmations...");
    for (int i = 0; i < 100; i++) {
        IoTHubDeviceClient_LL_DoWork(hub_handle);
        ThreadAPI_Sleep(SDK_DOWORK_DELAY_MS);
    }

    ESP_LOGI(TAG, "Telemetry phase complete.  %zu/%d messages queued successfully.",
             confirmed, TELEMETRY_MSG_COUNT);

cleanup:
    if (hub_handle != NULL) {
        IoTHubDeviceClient_LL_Destroy(hub_handle);
    }
    IoTHub_Deinit();
    platform_deinit();
}

// ─────────────────────────────────────────────────────────────────────────────
// app_main
// ─────────────────────────────────────────────────────────────────────────────
extern void optiga_trust_init(void);
void dps_task(void *pvParameter)
{
     if (custom_hsm_optiga_init() != 0) {
        ESP_LOGE(TAG, "custom_hsm_optiga_init() failed — cannot proceed.");
        return;
    }
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Phase 1: DPS Provisioning ---");
    if (!run_dps_provisioning()) {
        ESP_LOGE(TAG, "DPS provisioning failed.  Check ID Scope, Registration ID,");
        ESP_LOGE(TAG, "and that the device certificate CN matches the Registration ID.");
        return;
    }
    ESP_LOGI(TAG, "Provisioned OK → Hub: %s, Device: %s",
             s_dps.iothub_uri, s_dps.device_id);

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Phase 2: IoT Hub Telemetry ---");
    run_iothub_telemetry();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Sample complete.");
    custom_hsm_optiga_deinit();
    vTaskDelete(NULL);
}


void app_main()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " Azure DPS + OPTIGA™ Trust M Sample");
    ESP_LOGI(TAG, "========================================");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

      wifi_init_sta();

    ESP_LOGI(TAG, "Initialising OPTIGA Trust M...");
    optiga_trust_init();
    ESP_LOGI(TAG, "OPTIGA Trust M ready.");

    
    if (xTaskCreate(&dps_task,"dsp_task", 1024*10, NULL, 5, NULL) != pdPASS) {
        printf("Failed to create DPS task\n");
        return;
    }
}