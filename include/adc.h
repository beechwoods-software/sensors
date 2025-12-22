/*
 * Copyright 2023 Beechwoods Software, Inc brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */

#pragma once

/**
 * @file adc.h
 * @brief ADC sensor interface
 *
 * Provides a small API to initialize ADC channels configured through
 * devicetree and to read sensor levels.
 */

/**
 * @brief
 * Initialize the adc
 */
int bws_adc_init();

/**
 * @brief get the number of sensors
 * @return The number of sensors
 */
int bws_adc_get_num_sensors();
/**
 * @brief get the level of a sensor
 * @param num The sensor number
 * @return The value of the sensor
 */
int bws_adc_get_sensor(int num);

#ifdef CONFIG_USE_ADC_DEBOUNCE
/**
 * @brief dump the debounce data for a sensor
 * @param num The sensor number
 */
void bws_adc_dump_debounce(int num);
#endif
