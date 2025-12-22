/*
 * Copyright 2025 Beechwoods Software, Inc.  Brad Kemp
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */
#include <zephyr/logging/log.h>
#include "sensors_logging.h"
/** @brief register the log for the sensors module */
LOG_MODULE_REGISTER(SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL);

#
/**
 * @file sensor_log.c
 * @brief Sensor logging helper implementation
 *
 * Implements helpers used by sensor modules to log or dump diagnostic
 * information during development.
 */
