/*
 * Copyright 2023 Beechwoods Software Inc.
 * All Rights Reserved.
 * SPDX-License-Identifier: Apache 2.0
*/

#pragma once


/**
 * @file low_power.h
 * @brief Low power / sleep helper API
 *
 * Provides a thin API to put the system into light or deep sleep modes.
 */

typedef enum sleep_type {
  LIGHT_SLEEP,
  DEEP_SLEEP
}sleep_type_t;

/**
 * @brief Initialize low-power facilities
 * @return 0 on success, negative errno on failure
 */
int low_power_init();

/**
 * @brief Set the system sleep mode
 * @param sleep_type The sleep mode to enter
 * @return 0 on success, negative errno on failure
 */
int low_power_set(sleep_type_t sleep_type);

