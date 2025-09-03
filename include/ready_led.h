/*
 * Copyright 2023 Brad Kemp Beechwoods Software, Inc.
 * All Rights Reserved
 */

#pragma once

#ifdef CONFIG_USE_READY_LED

/**
 * @file ready_led.h
 * @brief the ready led is usually the builtin led on a device.
 * The ready led is used to signal to the user the status or fault on the device.
 * There are 6 speeds used to set the cadence of the LED blinking.
 * Some onboard leds will allow color to be set as well as blink speed
 */
#define READY_LED_DELAY_LONG  2000
#define READY_LED_DELAY_SHORT 1000
#define READY_LED_DELAY_QUICK  500
#define READY_LED_DELAY_PANIC  100
#define READY_LED_DELAY_ON       0
#define READY_LED_DELAY_OFF     -1

/**
 * @brief represent the differnt blink speeds
 *
 */
typedef enum ready_led_speed {
  READY_LED_LONG =  READY_LED_DELAY_LONG,  //< Blink speed of READY_LED_DELAY_LONG (2000) ms
  READY_LED_SHORT = READY_LED_DELAY_SHORT, //< Blink speed of READY_LED_DELAY_SHORT (1000) ms
  READY_LED_QUICK = READY_LED_DELAY_QUICK, //< Blink speed of READY_LED_DELAY_QUICK (500) ms
  READY_LED_PANIC = READY_LED_DELAY_PANIC, //< Blink speed of READY_LED_DELAY_PANIC (100) ms
  READY_LED_OFF   = READY_LED_DELAY_OFF,   //< Turn LED off
  READY_LED_ON    = READY_LED_DELAY_ON     //< Turn LED on
} ready_led_speed_t ;

/**
 * @brief initialize the ready led
 *
 * This function calls the led specific (GPIO, PWM, led strip, led controller, airoc) initialization function
 * @return 0 on success
 * @return -1 on error
 */
int ready_led_init();

/**
 * @brief turn the LED off
 *
 * This functions sets the delay speed to off and calls the led off worker function
 *
 * @return 0 on success
 * @return -errno on error
 */
int ready_led_off();

/**
 * @brief turn the LED on
 *
 * This function turns the delay speed to on and calls the led on worker function
 *
 * @return 0 on success
 * @return -errno on error
 */
int ready_led_on();

/**
 * @brief set the blink speed of the led
 *
 * This function sets the delay speed to the pass value.
 * The new delay speed is used at the end of the current delay
 * @return 0 on success
 * @return -errno on error
 */
void ready_led_set(ready_led_speed_t speed);
/**
 * @brief set the color of the LED
 * @return 0 on success
 * @return -1 on error;
 * @return 0 if LED does not support color
 */
int ready_led_color(uint32_t red, uint32_t green, uint32_t blue);

/**
 * @brief return the current delay setting
 *@retrun current delay setting
 */
ready_led_speed_t ready_led_speed();

#endif // CONFIG_USE_READY_LED

