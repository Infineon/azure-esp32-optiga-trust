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

#ifndef CUSTOM_HSM_OPTIGA_H
#define CUSTOM_HSM_OPTIGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// ---------------------------------------------------------------------------
// Initialise the custom HSM.
// Must be called once before prov_dev_security_init().
// Reads the device certificate from the OPTIGA Trust M chip into an internal
// buffer so it is available to the DPS/IoTHub client callbacks.
//
// Returns 0 on success, non-zero on failure.
// ---------------------------------------------------------------------------
int custom_hsm_optiga_init(void);

// ---------------------------------------------------------------------------
// De-initialise the custom HSM and free any resources.
// ---------------------------------------------------------------------------
void custom_hsm_optiga_deinit(void);

// ---------------------------------------------------------------------------
// Retrieve the PEM certificate that was read from OPTIGA at init time.
// The pointer is valid until custom_hsm_optiga_deinit() is called.
// ---------------------------------------------------------------------------
const char *custom_hsm_optiga_get_certificate(void);

// ---------------------------------------------------------------------------
// Retrieve the DPS Registration ID (= CN of the device certificate).
// Sourced from CONFIG_AZURE_DPS_REGISTRATION_ID (set via menuconfig).
// ---------------------------------------------------------------------------
const char *custom_hsm_optiga_get_common_name(void);

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_HSM_OPTIGA_H
