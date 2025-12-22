/*
  Copyright 2023 Beechwoods Software Inc.
  All Rights Reserved
*/

#ifdef CONFIG_USE_GPIO_LED

/**
 * @file gpio_led.c
 * @brief GPIO LED driver implementation
 *
 * Implements initialization and simple control helpers for LEDs wired
 * to GPIO pins using devicetree `gpio-leds` bindings.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/gpio.h>

#include "gpio_led.h"

#include <zephyr/logging/log.h>
#include "sensors_logging.h"
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL);

/* 
 * The gpio_leds are LEDS attached to GPIO pins
 * To configure the GPIO LEDS the boards/<board>.overlay file should contain something like this:
 * / {
 *  leds {
 *    compatible = "gpio-leds";
 *    red_leds: red_leds {
 *      gpios = <&gpio0 5 GPIO_ACTIVE_HIGH> ;
 *    };
 *    yellow_leds: yellow_leds {
 *      gpios = <&gpio0 18 GPIO_ACTIVE_HIGH> ;
 *    };
 *    green_leds:green_leds {
 *      gpios = <&gpio0 19 GPIO_ACTIVE_HIGH> ;
 *    };
 *  };
 * };
 *
 * The applications Kconfig file must contain the following:
 * config USE_GPIO_LED
 *      bool "Use GPIO LEDS"
 *      default n
 *      help
 *        Enable the use of GPIO LEDs
 *
 * The applications prj.conf or boards/<board>/conf needs to contain the following to enable GPIO LEDS
 * CONFIG_USE_GPIO_LED=y
 */

#define LED_OFF 0
#define LED_ON 1

     
static const struct gpio_dt_spec gpioleds[] = {
  DT_FOREACH_PROP_ELEM_SEP(DT_NODELABEL(gpio_leds), gpios, GPIO_DT_SPEC_GET_BY_IDX, (,))
};

#define NUM_GPIO_LEDS (sizeof(gpioleds)/sizeof(struct gpio_dt_spec))

#if 0
static const struct gpio_dt_spec leds_red =
  GPIO_DT_SPEC_GET(DT_NODELABEL(red_leds),gpios);
static const struct gpio_dt_spec leds_yellow =
  GPIO_DT_SPEC_GET(DT_NODELABEL(yellow_leds),gpios);
static const struct gpio_dt_spec leds_green =
  GPIO_DT_SPEC_GET(DT_NODELABEL(green_leds),gpios);
  //  GPIO_DT_SPEC_GET(RED_NODE, gpios);

static gpio_led_t leds;
#endif

int gpio_led_get_num_leds()
{
  return NUM_GPIO_LEDS;
}

int gpio_led_set_led(int led, bool value)
{
  int rc = -1;
  rc = gpio_pin_set_dt(&gpioleds[led], value);
  return rc;
}

void gpio_led_post() {
  int i;
  int rc;
  LOG_INF("gpio led post");
  for(i=0; i < NUM_GPIO_LEDS; i++) {
    if((rc = gpio_led_set_led(i, true)) < 0) {
      LOG_ERR("gpio %d post set failed %d", i, rc);
    }
  }
  k_msleep(1000);

  for(i=0; i < NUM_GPIO_LEDS; i++) {
    if((rc = gpio_led_set_led(i, false)) < 0) {
      LOG_ERR("gpio %d post clear failed %d", i, rc);
    }
  }
#if 0
  if(gpio_pin_set_dt(&leds_red, 1) < 0) {
    LOG_ERR("red led failed");
  }
  if(gpio_pin_set_dt(&leds_yellow, 1) < 0) {
    LOG_ERR("red yellow failed");
  }
  if(gpio_pin_set_dt(&leds_green, 1) < 0) {
    LOG_ERR("red green failed");
  }

  k_msleep(1000);
  
  if(gpio_pin_set_dt(&leds_red, 0) < 0) {
    LOG_ERR("red led failed");
  }
  if(gpio_pin_set_dt(&leds_yellow, 0) < 0) {
    LOG_ERR("yellow led failed");
  }
  if(gpio_pin_set_dt(&leds_green, 0) < 0) {
    LOG_ERR("green led failed");
  }
#endif
}

int
gpio_led_init()
{
  int i;
  for(i= 0; i< NUM_GPIO_LEDS; i++) {
    LOG_INF("Configuring LED %d on pin %d", i, gpioleds[i].pin);
    if(!device_is_ready(gpioleds[i].port)) {
      LOG_ERR("gpio LED %d not ready", i);
      return -1;
    }

    if(gpio_pin_configure_dt(&gpioleds[i], GPIO_OUTPUT_ACTIVE) < 0) {
      LOG_ERR("gpio LED %d  configure failed", i);
      return -1;
    }
  }
  return 0;
}

static gpio_color_zone_t mGpio_color_zone;
void
gpio_led_set_color_zone(gpio_color_zone_t zone)
{
  int i;
  for(i = 0; i < GPIO_ZONE_SIZE; i++) {
    mGpio_color_zone[i] = zone[1];
  }
}

#if 0
int
gpio_led_set_leds( int moisture_level)
{

  LOG_INF("setting moisture to %d",moisture_level);
  if (moisture_level >= mGpio_color_zone[4])
  {
    gpio_pin_set_dt(leds.leds_green,  LED_OFF);
    gpio_pin_set_dt(leds.leds_yellow, LED_OFF);
    gpio_pin_set_dt(leds.leds_red,    LED_ON);
  }
  else if (moisture_level >= mGpio_color_zone[3]) 
  {
    gpio_pin_set_dt(leds.leds_green,  LED_OFF);
    gpio_pin_set_dt(leds.leds_yellow, LED_ON);
    gpio_pin_set_dt(leds.leds_red,    LED_ON);
  }  
  else if (moisture_level >= mGpio_color_zone[2] )
  {
    gpio_pin_set_dt(leds.leds_green,  LED_OFF);
    gpio_pin_set_dt(leds.leds_yellow, LED_ON);
    gpio_pin_set_dt(leds.leds_red,    LED_OFF);
  }    
  else if (moisture_level >= mGpio_color_zone[1] )
  {
    gpio_pin_set_dt(leds.leds_green,  LED_ON);
    gpio_pin_set_dt(leds.leds_yellow, LED_ON);
    gpio_pin_set_dt(leds.leds_red,    LED_OFF);
  }
  else 
  {
    gpio_pin_set_dt(leds.leds_green,  LED_ON);
    gpio_pin_set_dt(leds.leds_yellow, LED_OFF);
    gpio_pin_set_dt(leds.leds_red,    LED_OFF);
  }
  return 0;
}  
#endif
#endif // CONFIG_GPIO_LED
