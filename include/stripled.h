/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */

#pragma once
#include <zephyr/drivers/led_strip.h>

/**
 * @file stripled.h
 * @brief LED strip (WS2812-style) helpers
 *
 * API for initializing and controlling an RGB LED strip using the
 * Zephyr `led_strip` driver interface.
 */

/**
 * @brief Initialize the LED strip subsystem.
 *
 * Configures and probes the underlying Zephyr `led_strip` device used to
 * drive WS2812-style RGB LEDs. Call this once during application startup.
 *
 * @return 0 on success, negative error code on failure.
 */
int strip_led_init(void);

/**
 * @brief Turn the entire strip on using the last set colors.
 *
 * @return 0 on success, negative error code on failure.
 */
int strip_led_on(void);

/**
 * @brief Turn the entire strip off (set all pixels to black).
 *
 * @return 0 on success, negative error code on failure.
 */
int strip_led_off(void);

/**
 * @brief Set a single pixel's color.
 *
 * The pixel index is 0-based. Color components are 0-255.
 * The change is applied to the driver's internal buffer; call
 * `strip_led_display()` to push the buffer to the strip.
 *
 * @param pixel 0-based pixel index
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @return 0 on success, negative error code on failure.
 */
int strip_led_set_color(int pixel, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Get number of LEDs managed by the strip driver.
 *
 * @return Number of LEDs (>0) or negative error code on failure.
 */
int strip_led_num_leds(void);

/**
 * @brief Display an array of RGB pixels on the strip.
 *
 * Copies `num_pixels` entries from `pixels` to the strip and updates
 * the LEDs. The `pixels` array must contain `struct led_rgb` entries.
 *
 * @param pixels Pointer to an array of `struct led_rgb` values.
 * @param num_pixels Number of pixels in the array.
 * @return 0 on success, negative error code on failure.
 */
int strip_led_display(struct led_rgb * pixels,  size_t num_pixels);
