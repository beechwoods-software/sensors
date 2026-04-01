/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */


#pragma once

#ifdef CONFIG_USE_MOTION_SENSOR
#/**
 * @file motion_sensor.h
 * @brief Motion sensor helper API
 *
 * Provides a tiny API to initialize a motion sensor and register a
 * callback invoked when motion is detected. The implementation may use
 * board devicetree bindings and GPIO interrupts; initialization should
 * validate device readiness and configure the interrupt handler.
 */

/**
 * @brief Motion sensor callback type
 *
 * The callback is invoked with no arguments when motion is detected.
 * Implementations should keep the handler lightweight and defer heavy
 * work to a work queue if needed.
 */
typedef void (*motion_sensor_handler_t)(void);

/**
 * @brief Initialize the motion sensor subsystem
 *
 * Configures device tree bindings and interrupt handlers required to
 * detect motion. The function is a no-op if `CONFIG_USE_MOTION_SENSOR`
 * is not enabled at build time.
 *
 * @param handler Optional callback to invoke when motion is detected.
 *                May be NULL; call `set_motion_sensor_callback` to
 *                change the handler after initialization.
 * @return 0 on success, negative errno on failure
 */
int motion_sensor_init(motion_sensor_handler_t handler);

/**
 * @brief Set or change the motion sensor callback
 *
 * This function updates the callback invoked on motion events.
 * Passing NULL will disable the callback.
 *
 * @param handler Callback function or NULL
 */
void set_motion_sensor_callback(motion_sensor_handler_t handler);
#endif // CONFIG_USE_MOTION_SENSOR
