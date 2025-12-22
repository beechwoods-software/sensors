/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */



#pragma once

#include <zephyr/input/input.h>

/**
 * @file ky40.h
 * @brief KY-040 rotary encoder helper
 *
 * Thin wrapper around Zephyr input events for handling the KY-040
 * rotary encoder and registering rotation/click callbacks.
 */

typedef const struct device * const ky40_device_t;

typedef void (*ky40_event_callback)(struct input_event *, int);

int ky40_init(ky40_event_callback func);
int ky40_get_rotation();
void ky40_set_callback(ky40_event_callback func, int index);
