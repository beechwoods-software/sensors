/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */


#pragma once

/**
 * @file meteorology.h
 * @brief Temperature, humidity and pressure sensor abstraction
 *
 * Provides helpers to initialize environmental sensors and read
 * temperature, humidity and pressure values.
 */


/**
 * @brief Initialize the environmental sensor subsystem.
 *
 * This function performs any hardware initialization and configuration
 * required to read temperature, humidity and pressure from the
 * available environmental sensor. It should be called once during
 * application startup.
 *
 * @return 0 on success, negative errno-style error code on failure.
 */
int meteorology_init(void);

/**
 * @brief Get the current temperature.
 *
 * The returned value is in degrees Celsius (°C). If the sensor cannot
 * provide a value the function returns NaN.
 *
 * @return Temperature in °C, or NaN on error.
 */
double meteorology_get_temperature(void);

/**
 * @brief Get the current relative humidity.
 *
 * The returned value is in percent relative humidity (%RH). If the
 * sensor cannot provide a value the function returns NaN.
 *
 * @return Relative humidity in %RH, or NaN on error.
 */
double meteorology_get_humidity(void);

/**
 * @brief Get the current barometric pressure.
 *
 * The returned value is in Pascals (Pa). If the sensor cannot provide a
 * value the function returns NaN.
 *
 * @return Pressure in Pa, or NaN on error.
 */
double meteorology_get_pressure(void);

