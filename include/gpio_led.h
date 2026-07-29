/*
 * Copyright 2023 Beechwoods Software Inc.
 * All Rights Reserved.
 * SPDX-License-Identifier: Apache 2.0
*/

#pragma once

#include <stdbool.h>

/**
 * @file gpio_led.h
 * @brief GPIO-based LED helper API
 *
 * Provides initialization and simple control primitives for LEDs
 * driven by GPIO lines. This header is conditionally compiled when
 * `CONFIG_USE_GPIO_LED` is enabled.
 */

/**
 * @def GPIO_ZONE_SIZE
 * @brief Number of entries in a color zone array.
 */
#define GPIO_ZONE_SIZE 6

/**
 * @typedef gpio_color_zone_t
 * @brief Array of integers representing different color zone thresholds.
 *
 * The array contains GPIO_ZONE_SIZE integers describing color thresholds
 * or levels. The first element (index 0) is the highest legal value and
 * the last element (index GPIO_ZONE_SIZE - 1) is the lowest legal value.
 */
typedef int gpio_color_zone_t[GPIO_ZONE_SIZE];

#ifdef CONFIG_USE_GPIO_LED

/**
 * @brief Post-initialization hook for the GPIO LED subsystem.
 *
 * Should be called after system boot or when bringing the LED subsystem
 * up from a suspended state. Typically used to restore LED state.
 */
void gpio_led_post(void);

/**
 * @brief Initialize the GPIO-based LED subsystem.
 *
 * Performs any hardware configuration required to manage LEDs via GPIO.
 *
 * @return 0 on success, negative error code on failure.
 */
int gpio_led_init(void);

/**
 * @brief Return number of LEDs managed by the GPIO LED driver.
 *
 * @return Number of LEDs (>=0) or a negative error code on failure.
 */
int gpio_led_get_num_leds(void);

/**
 * @brief Set the state of a GPIO-controlled LED.
 *
 * @param led Index of the LED (0-based).
 * @param value true to turn the LED on, false to turn it off.
 * @return 0 on success, negative error code on failure.
 */
int gpio_led_set_led(int led, bool value);

#endif /* CONFIG_USE_GPIO_LED */
