/*
 * Copyright Beechwoods Software, Inc. 2025 brad@beechwoods.com
 * All Rights Reserved
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include "sensors_logging.h"
#
/**
 * @file stripled.c
 * @brief LED strip driver implementation
 *
 * Implements the high-level control functions for an addressable RGB
 * LED strip (initialization, per-pixel color setting and display).
 */
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

#ifdef CONFIG_USE_STRIP_LED
/*
 * A strip LED is a set of one or more LEDS driven by a single GPIO. The strip LED uses I2C or SPI to
 * send an array of 4 octet commands. Each element in the array corrosponds to one LED.
 * / {
 *     zephyr,user {
 *      stripleds = <&led_strip> 
 *     };
 * };
 * &spi2 {
 *	#address-cells = <1>;
 *	#size-cells = <0>;
 *	status = "okay";
 *	pinctrl-0 = <&spim2_default>;
 *	pinctrl-names = "default";
 *
 *	line-idle-low;
 *	status = "okay";
 *
 *	led_strip: ws2812@0 {
 *		compatible = "worldsemi,ws2812-spi";
 *
 *		reg = <0>; 
 *		spi-max-frequency = <6400000>;
 *
 *		chain-length = <8>; 
 *		spi-cpha;
 *		spi-one-frame = <0xf0>;
 *		spi-zero-frame = <0xc0>; 
 *		color-mapping = <LED_COLOR_ID_GREEN
 *					LED_COLOR_ID_RED
 *					LED_COLOR_ID_BLUE>;
 *	};
 * };
 * &pinctrl {
 *	spim2_default: spim2_default {
 *		group2 {
 *			pinmux = <SPIM2_MOSI_GPIO13>;
 *			output-low;
 *		};
 *    };
 * };
 *
 */

#define ZEPHYR_USER zephyr_user
#define LED_USER_NAME stipleds
#define ZEPHYR_USER_NODE DT_PATH(ZEPHYR_USER)

#define LED_NODE_ID	 DT_PROP(ZEPHYR_USER_NODE, stripleds )
#define LED_DEVICE_ID    DT_PARENT(DT_PROP(ZEPHYR_USER_NODE, LED_USER_NAME ))
#if DT_NODE_HAS_PROP(LED_NODE_ID, chain_length)
#else
#error Unable to determine length of LED strip x LED_NODE_ID x
#endif


#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

#define STRIP_NUM_PIXELS	DT_PROP(LED_NODE_ID, chain_length)
static const struct device *const strip = DEVICE_DT_GET(LED_NODE_ID);

static struct led_rgb pixels_off = RGB(0x00, 0x00, 0x00);
static struct led_rgb pixels_on = RGB(0xff, 0xff, 0xff) ;

static struct led_rgb pixelbuffer[STRIP_NUM_PIXELS];


int strip_led_init()
{
  if (!device_is_ready(strip)) {
    LOG_ERR("LED strip deviec %s is not ready", strip->name);
    return -1;
  }
  return 0;
}

int strip_led_on()
{
  int i;
  for(i = 0; i < STRIP_NUM_PIXELS; i++) {
    pixelbuffer[i] = pixels_on;
  }
  return led_strip_update_rgb(strip, pixelbuffer, STRIP_NUM_PIXELS);
}

int strip_led_off()
{
  int i;
  for(i = 0; i < STRIP_NUM_PIXELS; i++) {
    pixelbuffer[i] = pixels_off;
  }
  return led_strip_update_rgb(strip, pixelbuffer, STRIP_NUM_PIXELS);
}

int strip_led_set_color(int pixel,  uint8_t red, uint8_t green, uint8_t blue)
{
  pixels_on.r = red;
  pixels_on.g = green;
  pixels_on.b = blue;

  return 0;
}

int strip_led_num_leds()
{
  return STRIP_NUM_PIXELS;
}

int strip_led_display(struct led_rgb * pixels,  size_t num_pixels)
{
  if((num_pixels > STRIP_NUM_PIXELS) || (num_pixels < 0)) {
    return -EINVAL;
  }
  return led_strip_update_rgb(strip, pixels, num_pixels);
}

#endif // CONFIG_USE_STRIP_LED
