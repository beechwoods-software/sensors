/*
 * Copyright 2023 Beechwoods Software, Inc.  Brad Kemp
 * All Rights Reserved
 * SPDX-License-Identifier: Apache 2.0
 */



#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/led_strip.h>
#include "ready_led.h"
#include <zephyr/logging/log.h>
#include "sensors_logging.h"
LOG_MODULE_DECLARE(SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL);

#ifdef CONFIG_USE_READY_LED
/**
 * @file ready_led.c
 * @brief the ready LED implementation
 * @details
 * The ready led is a LED used to show status. It is useful when a device is running
 * without a console. It is useually the led that is soldered to the PCB of the microprocessor
 *
 * Implements the ready LED abstraction used to indicate device status
cd * (blink rates, color when supported) for board-level feedback.
 * The ready_led sensor supports 5 types of LEDs
 * 1) gpio LEDs: these are single color LEDs driven by a GPIO line (CONFIG_USE_READY_LED_GPIO)
 * 2) LED strips: These are one or more LEDs driven by a GPIO line, they are capable of multiple colors (CONFIG_USE_READY_LED_STRIP)
 * 3) PWM LEDS: These are LEDs driven by PWM GPIOS, usally consisting of 3 GPIO lines (CONFIG_USE_READY_LED_PWM)
 * 4) LED Driver: This is support for the Zephyr LED driver. It support gpio LEDS and some leds strips the underlying driver must support the LED controller device (CONFIG_USE_READY_LED_CONTROLLER)
 * 5) Raspberry PI Pico W: This is a big of a hack to support the onboard LED on the Raspberry PI Pico W (CONFIG_USE_READY_LED_PICO_W)
 *
 *
 * For GPIO ready leds
 * To configure the ready_led, the boards/\<board\>.overlay should look something like this:
 * @code
 * / {
 *    leds {
 *      compatible = "gpio-leds";
 *      onboard_led: onbaord_led {
 *         gpios = <&gpio0 2 ( GPIO_ACTIVE_HIGH) > ;
 *      };
 *   };
 *   zephyr,user {
 *      ready-led = <&onboard_led>;
 *   };
 * };
 * @endcode
 *
 * The ready LED is configured via Kconfig. 
 *
 * And the applications prj.conf file should have these configurations
 * CONFIG_USE_READY_LED=y
 * CONFIG_USE_READY_LED_GPIO=y (this is the default)
 *
 * for pwm ready leds
 * To configure the ready_led, the boards/\<board\>.overlay should look something like this:
 * @code
 * / {
 *     pwmleds {
 *       compatible = "pwm-leds";
 *       pwm_led: pwm_led {
 *           pwms = <&ledc0 0 10000 PWM_POLARITY_NORMAL >,
 *               <&ledc0 1 10000 PWM_POLARITY_NORMAL >,
 *               <&ledc0 2 10000 PWM_POLARITY_NORMAL >;
 *       };
 *   };
 *   zephyr,user {
 *      ready-led = <&pwm_led_led>;
 *   };
 * };
 * &pinctrl {
 *	 ledc0_default: ledc0_default {
 *	 		group1 {
 *				pinmux = <LEDC_CH0_GPIO15>,
 *					 <LEDC_CH1_GPIO12>,
 *					 <LEDC_CH2_GPIO14>;
 *				output-enable;
 *			};
 *	};
 * &ledc0 {
 *   pinctrl-0 = <&ledc0_default>;
 *   pinctrl-names = "default";
 *   status = "okay";
 *   #address-cells = <1>;
 *   #size-cells = <0>;
 *   channle0@0 {
 *       reg = <0x0>;
 *		timer = <0>;
 *   };
 *   channle0@1 {
 *       reg = <0x1>;
 *		timer = <0>;
 *	 };
 *   channle0@2 {
 *       reg = <0x2>;
 *		timer = <0>;
 *	 };
 * };
 * @endcode
 *
 * The ready LED is configured via Kconfig. 
 *
 * And the applications prj.conf file should have these configurations
 * CONFIG_USE_READY_LED=y
 * CONFIG_USE_READY_LED_PWM=y
 * 
 *
 * for LED strip ready leds
 * The ready led module does not allow for indiviual colors on multi-pixel LEDS.
 * The ready led color values  are limited to a uint8_t (0-255 )
 * To configure the ready_led, the boards/\<board\>.overlay should look something like this:
 * @code
 * / {
 *   zephyr,user {
 *      ready-led = <&led_strip>;
 *   };
 * };
 *
 * &spi2 {
 *	#address-cells = <1>;
 *	#size-cells = <0>;
 *	status = "okay";
 *	pinctrl-0 = <&spim2_default>;
 *	pinctrl-names = "default";
 *
 *	line-idle-low;
 *	status = "okay";

 *	led_strip: ws2812@0 {
 *		compatible = "worldsemi,ws2812-spi";
 *
 *
 *		reg = <0>; 
 *		spi-max-frequency = <6400000>;
 *
 *		chain-length = <1>; 
 *		spi-cpha;
 *		spi-one-frame = <0xf0>; 
 *		spi-zero-frame = <0xc0>; 
 *		color-mapping = <LED_COLOR_ID_GREEN
 *					LED_COLOR_ID_RED
 *					LED_COLOR_ID_BLUE>;
 *	 };
 * };
 * &pinctrl {
 *	 spim2_default: spim2_default {
 *		group2 {
 *			pinmux = <SPIM2_MOSI_GPIO13>;
 *			output-low;
 *		};
 *   };
 * };
 *
 * @endcode
 */

#define ZEPHYR_USER zephyr_user
#define ZEPHYR_USER_NODE DT_PATH(ZEPHYR_USER)

#ifndef CONFIG_USE_READY_LED_PICO_W
#if !DT_NODE_EXISTS(DT_PATH(ZEPHYR_USER)) ||            \
	!DT_NODE_HAS_PROP(DT_PATH(ZEPHYR_USER), ready_led)
#error "No suitable devicetree overlay specified for ready_led"
#endif
#endif

#define LED_NODE_ID	 DT_PROP(ZEPHYR_USER_NODE,ready_led)
#define LED_DEVICE_ID DT_PARENT(DT_PROP(ZEPHYR_USER_NODE,ready_led))


#define LED_OFF 0
#define LED_ON 1


K_SEM_DEFINE(ready_led_sem, 0, 1);

volatile ready_led_speed_t  gReadyLedDelay = READY_LED_DELAY_OFF;
static void process_ready_led(void);
K_THREAD_DEFINE(ready_led_thread, CONFIG_READY_LED_STACK_SIZE,
		process_ready_led, NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, -1);

/**
 * @struct ready_led_vtable
 * A virtual function table for handling different LED technologies
 */
typedef struct ready_led_vtable {
  int (*init)();  /*!< Initialize the LED */
  int (*on)();    /*!< Turn the LED on */
  int (*off)();   /*!< Turn the LED off */
  int (*set_color)(int red, int green, int blue); /*!< Set the color of the LED */
} ready_led_vtable_t;
ready_led_speed_t ready_led_speed()
{
  return gReadyLedDelay;
}

#ifdef CONFIG_USE_READY_LED_GPIO
static const struct gpio_dt_spec ready_led =
  GPIO_DT_SPEC_GET(LED_NODE_ID,gpios);
static int ready_led_gpio_init()
{
    if(!device_is_ready(ready_led.port)) {
    LOG_ERR("Ready LED not ready");
    return -1;
  }
  if(gpio_pin_configure_dt(&ready_led, GPIO_OUTPUT_INACTIVE) < 0) {
    LOG_ERR("Unable to configure ready LED");
    return -1;
  }
  LOG_INF("Configuring Ready Led on gpio %d", ready_led.pin);
  return 0;
}
int ready_led_gpio_on()
 {
  return gpio_pin_set_dt(&ready_led, LED_ON);
};
int ready_led_gpio_off()
{
    return gpio_pin_set_dt(&ready_led, LED_OFF);
}
int ready_led_gpio_set_color(int red, int green, int blue)
{
  LOG_ERR("Ready_led gpio does not implement setting color");
  return 0;
}
static ready_led_vtable_t vtable_gpio = {
  .init = ready_led_gpio_init,
  .on = ready_led_gpio_on,
  .off = ready_led_gpio_off,
  .set_color = ready_led_gpio_set_color
};
#endif // CONFIG_USE_READY_LED_GPIO

#ifdef CONFIG_USE_READY_LED_PWM
struct ready_pwm {
  const struct pwm_dt_spec dt_spec;
  uint32_t pulse;
};
#define PWMLED_DT_SPEC_GET_BY_IDX(node_id, prop, idx) \
  {                                                   \
    .dt_spec = PWM_DT_SPEC_GET_BY_IDX(node_id, idx),  \
    .pulse = 0,                                     \
   },

static struct ready_pwm readyleds[] = {
  DT_FOREACH_PROP_ELEM(LED_NODE_ID, pwms, PWMLED_DT_SPEC_GET_BY_IDX)
};
#define NUM_READY_LEDS (sizeof(readyleds)/sizeof(struct ready_pwm))


int ready_led_pwm_init()
{

  int i;
  for(i = 0 ; i < NUM_READY_LEDS; i++) {
    if(!device_is_ready(readyleds[i].dt_spec.dev)) {
      LOG_ERR("ready led pwm %d is not ready", i);
      return -1;
    }
    readyleds[i].pulse = readyleds[i].dt_spec.period;
    LOG_INF("Ready LED PWM %d initialized period %d pulse %d", i, readyleds[i].dt_spec.period,readyleds[i].pulse);
  }

  return 0;
}

int ready_led_pwm_on()
{
  int rc = -1;
  int i;
  LOG_DBG("setting %d leds (%d, %d, %d)", NUM_READY_LEDS, readyleds[0].pulse, readyleds[1].pulse, readyleds[2].pulse);
  for(i = 0; i < NUM_READY_LEDS; i++) {
    rc = pwm_set_pulse_dt(&readyleds[i].dt_spec, readyleds[i].pulse);
    if(rc < 0) {
      LOG_ERR("setting %d pulse to %d failed %d", i,  readyleds[i].pulse, rc);
      return rc;
    }
  }
  LOG_DBG("Ready Led PWM set color successful");
  return rc;
}
int ready_led_pwm_off()
{
  int rc;
  int i;
  for(i = 0; i < NUM_READY_LEDS; i++) {
    rc = pwm_set_pulse_dt(&readyleds[i].dt_spec, 0);
    if(rc < 0) {
      LOG_ERR("setting %d pulse to 0 failed %d", i, rc);
      return rc;
    }
  }
  return rc;
}

int ready_led_pwm_set_color(int red, int green, int blue)
{
  int rc = 0;

  if(red > readyleds[0].dt_spec.period) {
    LOG_WRN("Red to large %d setting to %d", red, readyleds[0].dt_spec.period);
    red = readyleds[0].dt_spec.period;
  }
  if(green > readyleds[1].dt_spec.period) {
    LOG_WRN("Green to large %d setting to %d", green, readyleds[1].dt_spec.period);
    green = readyleds[1].dt_spec.period;
  }
  if(blue > readyleds[2].dt_spec.period) {
    LOG_WRN("Blue to large %d setting to %d", blue, readyleds[2].dt_spec.period);
    blue = readyleds[2].dt_spec.period;
  }
  readyleds[0].pulse = red;
  readyleds[1].pulse = green;
  readyleds[2].pulse = blue;
  return rc;
}

static ready_led_vtable_t vtable_pwm = {
  .init = ready_led_pwm_init,
  .on = ready_led_pwm_on,
  .off = ready_led_pwm_off,
  .set_color = ready_led_pwm_set_color,
};

#endif // CONFIG_USE_READY_LED_PWM


#ifdef CONFIG_USE_READY_LED_STRIP

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

#if DT_NODE_HAS_PROP(LED_NODE_ID, chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define STRIP_NUM_PIXELS	DT_PROP(LED_NODE_ID, chain_length)
static const struct device *const strip = DEVICE_DT_GET(LED_NODE_ID);

static struct led_rgb pixels_off = RGB(0x00, 0x00, 0x00);
static struct led_rgb pixels_on = RGB(0x00, 0x0f, 0x00) ;

static struct led_rgb pixelbuffer[STRIP_NUM_PIXELS];


int ready_led_strip_init()
{
  if (!device_is_ready(strip)) {
    LOG_ERR("LED strip deviec %s is not ready", strip->name);
    return -1;
  }
  return 0;
}
int ready_led_strip_on()
{
  int i;
  for(i = 0; i < STRIP_NUM_PIXELS; i++) {
    pixelbuffer[i] = pixels_on;
  }
  return led_strip_update_rgb(strip, pixelbuffer, STRIP_NUM_PIXELS);
}

int ready_led_strip_off()
{
  int i;
  for(i = 0; i < STRIP_NUM_PIXELS; i++) {
    pixelbuffer[i] = pixels_off;
  }
  return led_strip_update_rgb(strip, pixelbuffer, STRIP_NUM_PIXELS);
}

int ready_led_strip_set_color(int red, int green, int blue)
{
  if(red > 255) {
    LOG_WRN("red to large %d setting to 255", red);
    red = 255;
  }
  if(green > 255) {
    LOG_WRN("green to large %d setting to 255", green);
    green = 255;
  }
  if(blue > 255) {
    LOG_WRN("blue to large %d setting to 255", blue);
    blue = 255;
  }
  pixels_on.r = red;
  pixels_on.g = green;
  pixels_on.b = blue;

  return 0;
}

static ready_led_vtable_t vtable_strip = {
  .init = ready_led_strip_init,
  .on = ready_led_strip_on,
  .off = ready_led_strip_off,
  .set_color = ready_led_strip_set_color,
};


#endif // CONFIG_USE_READY_LED_STRIP

#ifdef USE_READY_LED_CONTROLLER

#define _COLOR_MAPPING(led_node_id)				\
const uint8_t led_color_mapping_##led_node_id[] =		\
	DT_PROP(led_node_id, color_mapping)

#define COLOR_MAPPING(led_node_id)					\
	IF_ENABLED(DT_NODE_HAS_PROP(led_node_id, color_mapping),	\
		   (_COLOR_MAPPING(led_node_id);))

#define LED_INFO_COLOR(led_node_id)				\
{								\
	.label		= DT_PROP(led_node_id, label),		\
	.index		= DT_PROP_OR(led_node_id, index, 0),	\
	.num_colors	=					\
		DT_PROP_LEN(led_node_id, color_mapping),	\
      .color_mapping	= led_color_mapping_##led_node_id,	\
},

#define LED_INFO_NO_COLOR(led_node_id)				\
{								\
	.label		= DT_PROP(led_node_id, label),		\
	.index		= DT_PROP_OR(led_node_id, index, 0),	\
	.num_colors	= 0,					\
	.color_mapping	= NULL,					\
},

#define LED_INFO(led_node_id)	\
	COND_CODE_1(DT_NODE_HAS_PROP(led_node_id, color_mapping),	\
		    (LED_INFO_COLOR(led_node_id)),			\
		    (LED_INFO_NO_COLOR(led_node_id)))

#define LED_CONTROLLER_INFO(node_id)				\
								\
DT_FOREACH_CHILD(node_id, COLOR_MAPPING)			\
								\
const struct led_info led_led_info[] = {			\
	DT_FOREACH_CHILD(node_id, LED_INFO)			\
};								\
								\
static  int num_leds = ARRAY_SIZE(led_led_info)

LED_CONTROLLER_INFO(LED_NODE_ID);

static const struct led_dt_spec ready_led =  LED_DT_SPEC_GET(LED_NODE_ID);


int ready_led_controller_init()
{
  LOG_DBG("ready led dev %s %d %p", ready_led.dev->name, ready_led.index, ready_led.dev->config);
  if(!led_is_ready_dt(&ready_led)) {
    LOG_ERR("Ready LED not ready");
    return -1;
  }
  return 0;
}

int ready_led_controller_off()
{
  return  led_off_dt(&ready_led);
}
int ready_led_controller_on()
{
  return  led_on_dt(&ready_led);
}
ready_led_vtable_t vtable_airoc = {
  .init = ready_led_controller_init,
  .on = ready_led_controller_on,
  .off = ready_led_controller_off,
  .set_color = ready_led_controller_set_color
};
#endif // USE_READY_LED_CONTROLLER

#ifdef CONFIG_USE_READY_LED_PICO_W

#ifdef CONFIG_WIFI_AIROC
int airoc_led_gpio_on();
int airoc_led_gpio_off();
#endif // ifdef CONFIG_WIFI_AIROC

int ready_led_pico_init()
{
  return 0;
}

int ready_led_pico_on()
{
#ifdef CONFIG_WIFI_AIROC	
  return airoc_led_gpio_on();
#endif // ifdef CONFIG_WIFI_AIROC
  return 0;
}
int ready_led_pico_off()
{
#ifdef CONFIG_WIFI_AIROC	
  return airoc_led_gpio_off();
#endif // ifdef CONFIG_WIFI_AIROC
  return 0;
}
int ready_led_pico_set_color(int red, int green, int blue)
{
  LOG_WRN("Airoc led does not support setting color");
  return 0;
}

ready_led_vtable_t vtable_airoc = {
  .init = ready_led_pico_init,
  .on = ready_led_pico_on,
  .off = ready_led_pico_off,
  .set_color = ready_led_pico_set_color
};
#endif // CONFIG_USE_READY_LED_PICO_W

ready_led_vtable_t *vtable =
#ifdef CONFIG_USE_READY_LED_GPIO
  &vtable_gpio
#endif // CONFIG_USE_READY_LED_GPIO
#ifdef CONFIG_USE_READY_LED_PWM
  &vtable_pwm
#endif // CONFIG_USE_READTY_LED_PWM
#ifdef CONFIG_USE_READY_LED_STRIP
  &vtable_strip
#endif // CONFIG_USE_READY_LED_STRIP
#ifdef CONFIG_USE_READY_LED_CONTROLLER
  &vtable_controller
#endif // CONFIG_USE_READY_LED_CONTROLLER
#ifdef CONFIG_USE_READY_LED_PICO_W
  &vtable_airoc
#endif // CONFIG_USE_READY_LED_PICO_W
  ;


int
ready_led_init()
{
  int rc;
  LOG_INF("Ready_led init");
  rc = vtable->init();
  if(rc != 0) {
    LOG_ERR("Ready led init failed %d",rc);
    return rc;
  }
  k_thread_start(ready_led_thread);

  return ready_led_on();
}

static int
_ready_led_off()
{
  int rc = 0;
  LOG_DBG("ready_led off");
  rc = vtable->off();
  if(rc) {
    LOG_ERR("ready Off Failed");
  }
  return rc;
}
int ready_led_off()
{
  gReadyLedDelay = READY_LED_OFF;
  return ( _ready_led_off());
}


static int
_ready_led_on()
{
  int rc = 0;
  LOG_DBG("ready_led on %p", vtable->on);
  rc = vtable->on();
  if(rc) {
    LOG_ERR("setting ready led on failed %d",rc);
  }
  return rc;
}

int
ready_led_on()
{
  gReadyLedDelay = READY_LED_ON;
  return _ready_led_on();
}



/**
 * @brief set the speed of the LED blink
 *
 * @param speed sets the duration of the blink using an enum with the values of
 *       READY_LED_DELAY_LONG  2000ms
 *       READY_LED_DELAY_SHORT 1000ms
 *       READY_LED_DELAY_QUICK  500ms
 *       READY_LED DELAY_PANIC  100ms
 *       READY_LED_DELAY_OFF    off
 *       READY_LED_DELAY_ON     on
 */
void
ready_led_set(ready_led_speed_t speed)
{

  switch (speed) {
  case READY_LED_LONG:
    gReadyLedDelay = READY_LED_DELAY_LONG;
    break;
  case READY_LED_SHORT:
    gReadyLedDelay = READY_LED_DELAY_SHORT;
    break;
  case READY_LED_QUICK:
    gReadyLedDelay = READY_LED_DELAY_QUICK;
    break;
  case READY_LED_PANIC:
    gReadyLedDelay = READY_LED_DELAY_PANIC;
    break;
  case READY_LED_OFF:
    gReadyLedDelay = READY_LED_DELAY_OFF;
    break;
  case READY_LED_ON:
    gReadyLedDelay = READY_LED_DELAY_ON;
    break;
  default:
    LOG_ERR("Ready_led unknown speed %d", speed);
    gReadyLedDelay = speed;
    break;
  }
  k_sem_give(&ready_led_sem);
}
void
process_ready_led(void)
{

  while(1) {
    while((READY_LED_DELAY_OFF == gReadyLedDelay) ||
          (READY_LED_DELAY_ON == gReadyLedDelay)) {
      k_sem_take(&ready_led_sem, K_FOREVER);
    }
    if(READY_LED_DELAY_OFF != gReadyLedDelay) {
      _ready_led_on();
      k_msleep(gReadyLedDelay);
    }
    if(READY_LED_DELAY_ON != gReadyLedDelay) {
      _ready_led_off();
    }
    k_msleep(gReadyLedDelay);
  }
}
int ready_led_color(uint32_t red, uint32_t green, uint32_t blue)
{

  return vtable->set_color(red, green,blue);
}

int ready_led_num_leds()
{
#ifdef USE_READY_LED_CONTROLLER
  return num_leds;
#else
  return 0;
#endif

}

#endif // CONFIG_USE_READY_LED

