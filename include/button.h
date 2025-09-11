/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */

#pragma once
/**
 * @file button.h
 * @brief This file contains the defintions for button control
 */

/**
 * @brief defines the state of the button
 */
typedef enum button_state {
  BUTTON_STATE_PRESSED,  /**< The button has been pressed */
  BUTTON_STATE_RELEASED  /**< The button has been released */
} button_state_t;

/**
 * @brief Represents the state of a button when callback occurs
 */
typedef struct button_callback_data {
  int button; /**< The button number */
  button_state_t state; /**< The state of the button @see button_state_t */
} button_callback_data_t;
/**
 * @brief callback function invoked when a button is pressed or released
 * @param bcd Pointer to a button_callback_data structure for the button
 */
typedef void (*button_state_handler_t)(button_callback_data_t * bcd);

/**
 * @brief initialize the button
 * @param handler the callback funtion invoked when the button is pressed or released
 * @return 0 on Success
 * @return -1 on failure
 */
int button_init(button_state_handler_t handler);

/**
 * @brief set the callback function invoked when the button is pressed or released*
 * @param cb the callback funtion invoked when the button is pressed or released
 */
void set_button_callback( button_state_handler_t cb);
