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
 

#include "custom_hsm_optiga.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "esp_log.h"
#include "stdbool.h"

// Project Kconfig
#include "sdkconfig.h"

#include "azure_prov_client/prov_security_factory.h"
#include "hsm_client_data.h"

static const char *TAG = "custom_hsm_optiga";


#define CERT_BUFFER_SIZE  1300

static char   s_cert_pem[CERT_BUFFER_SIZE];
static bool   s_initialised = false;

//Dummy private key assigned to allocate buffer
static const char* DUMMY_EC_KEY =
"-----BEGIN EC PRIVATE KEY-----\r\n"
"MHcCAQEEIDkoXdx0gErvQDm8TyDoqWJibRSo0GWNCjWR6oMjKUhRoAoGCCqGSM49\r\n"
"AwEHoUQDQgAEU7b1qd5vN0sCxEn8On2uoFEkD9c9APP1rOT/JPinjkASxzpbpxgp\r\n"
"bjBkpNh8Or8AmGboQjRUnFgGUA+AB6OoQg==\r\n"
"-----END EC PRIVATE KEY-----\r\n";
// ---------------------------------------------------------------------------
// Public init / deinit
// ---------------------------------------------------------------------------
extern void read_certificate_from_optiga(char * cert_pem, uint16_t * cert_pem_length);
int custom_hsm_optiga_init(void)
{
    if (s_initialised) {
        return 0;
    }

    ESP_LOGI(TAG, "Reading device certificate from OPTIGA Trust M (slot %d)...",
             CONFIG_OPTIGA_TRUST_M_CERT_SLOT);

    memset(s_cert_pem, 0, sizeof(s_cert_pem));
    uint16_t cert_len = sizeof(s_cert_pem);    
    read_certificate_from_optiga(s_cert_pem, &cert_len);
    if (cert_len <= 0) {
        ESP_LOGE(TAG, "Failed to read certificate from OPTIGA (ret=%d).", cert_len);
        return -1;
    }

    ESP_LOGI(TAG, "Certificate read OK (%d bytes).", cert_len);
    ESP_LOGI(TAG, "Certificate:\n%s", s_cert_pem);

    s_initialised = true;
    return 0;

}

void custom_hsm_optiga_deinit(void)
{
    memset(s_cert_pem, 0, sizeof(s_cert_pem));
    s_initialised = false;
}

const char *custom_hsm_optiga_get_certificate(void)
{
    return s_initialised ? s_cert_pem : NULL;
}

const char *custom_hsm_optiga_get_common_name(void)
{
    // Must match the CN of the certificate stored in OPTIGA for individual
    // enrollment.  Configured via idf.py menuconfig → Example Configuration.
    return CONFIG_AZURE_DPS_REGISTRATION_ID;
}

typedef struct CUSTOM_HSM_INFO_TAG {
    int placeholder; // No heap state needed; everything is in s_cert_pem.
} CUSTOM_HSM_INFO;

static HSM_CLIENT_HANDLE custom_hsm_create(void)
{
    CUSTOM_HSM_INFO *result = (CUSTOM_HSM_INFO *)malloc(sizeof(CUSTOM_HSM_INFO));
    if (result == NULL) {
        ESP_LOGE(TAG, "custom_hsm_create: malloc failed.");
    }
    return (HSM_CLIENT_HANDLE)result;
}

static void custom_hsm_destroy(HSM_CLIENT_HANDLE handle)
{
    free(handle);
}

static char *custom_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
    (void)handle;
    if (!s_initialised) {
        ESP_LOGE(TAG, "custom_hsm_get_certificate called before init.");
        return NULL;
    }
    // The SDK takes ownership of this buffer and calls free() on it.
    // We must return a malloc'd copy.
    char *cert_copy = (char *)malloc(strlen(s_cert_pem) + 1);
    if (cert_copy != NULL) {
        strcpy(cert_copy, s_cert_pem);
    }
    return cert_copy;
}

// ── Private key ─────────────────────────────────────────────────────────────
static char *custom_hsm_get_key(HSM_CLIENT_HANDLE handle)
{
    (void)handle;
    // // Return malloc'd empty string so the SDK can call free() safely.
    char *copy = (char *)malloc(strlen(DUMMY_EC_KEY) + 1);
    strcpy(copy, DUMMY_EC_KEY);
    return copy;
}

// ── Common name (= DPS Registration ID) ─────────────────────────────────────

static char *custom_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{    (void)handle;
    const char *cn = CONFIG_AZURE_DPS_REGISTRATION_ID;
    char *cn_copy = (char *)malloc(strlen(cn) + 1);
    if (cn_copy != NULL) {
        strcpy(cn_copy, cn);
    }
    return cn_copy;
}


int hsm_client_x509_init(void)
{
    ESP_LOGI(TAG, "hsm_client_x509_init() called by SDK");
    return 0;
}

void hsm_client_x509_deinit(void)
{
    ESP_LOGI(TAG, "hsm_client_x509_deinit() called by SDK");
}


static const HSM_CLIENT_X509_INTERFACE s_x509_interface = {
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_get_certificate,
    custom_hsm_get_key,
    custom_hsm_get_common_name,
};

// This function is called by the azure-iot-sdk-c when the custom HSM library
// is linked.  It returns the vtable for X.509 operations.
const HSM_CLIENT_X509_INTERFACE *hsm_client_x509_interface(void)
{
    return &s_x509_interface;
}
