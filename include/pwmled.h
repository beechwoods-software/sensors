/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */

#pragma once

#include <stdint.h>

/**
 * @file pwmled.h
 * @brief PWM driven LED helpers
 *
 * Control functions for LEDs driven by PWM channels (set period, pulse,
 * and simple helpers to map sensor values to LED output).
 */

int pwmled_init();
int pwmled_post();
int pwmled_set_pulse(int led_num, uint32_t pulse);
uint32_t pwmled_get_pulse(int led_num);
int pwmled_set_period(int led_num, uint32_t period);
uint32_t pwmled_get_period();
int pwmled_set(int led_num, uint32_t period, uint32_t pulse);
uint32_t pwmled_get_max_period();
int pwmled_get_num_leds();

/**
 * deprecated
 */
int pwmled_set_leds( int moisture_level);
void pwmled_set_rg_max(int maximum);
