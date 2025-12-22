/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */
#ifdef CONFIG_USE_MOTION_SENSOR


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <stdio.h>
#include "motion_sensor.h"

#include <zephyr/logging/log.h>
#include "sensors_logging.h"
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

/*
 * This motion sensor supports the RCWL-0516 Microwave radar motion sensor and the AM312 infrared motion sensor
 * to configure the sensor the boards/<board>.overlay file shold have something like this
 * / {
 *    motion_sensors {
 *       compatible = "gpio-keys";
 *       motionsensor: motionsensor {
 *  	    gpios = < &gpio0 27 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH) >;
 *          label = "motion sensor";
 *       };	   
 *   };
 *};
 * for the esp32c3_devkitm the motion sensor is connected to GPIO 12
 *    motion_sensors {
 *       compatible = "gpio-keys";
 *  	motionsensor: motionsensor {
 *  	    gpios = < &gpio0 12 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH) >;
 *	    label = "motion sensor";
 *       };
 *   };
 *
 * The motion sensor is configured via Kconfig. Your Kconfig file should contain this
 * config USE_MOTION_SENSOR
 *      bool "Use the motion sensor"
 *      default n
 *      help
 *        Configure a motion sensor for your project
 * The prj.conf file of your application must contain
 * CONFIG_USE_MTION_SENSOR=y
 *
 * motion_sensor_init is called to initialize the motion sensor. the paramter handler is a 
 * pointer to a function to handle callbacks when motion is detected, it may be NULL
 */

static const struct gpio_dt_spec motion_sensor = GPIO_DT_SPEC_GET(DT_NODELABEL(motionsensor), gpios);
static struct gpio_callback motion_cb_data;
static motion_sensor_handler_t motion_callback;

static void motion_cooldown(struct k_work * work)
{
  if (NULL != motion_callback) {
    motion_callback();
  }
}

K_WORK_DELAYABLE_DEFINE(motion_work, motion_cooldown);
/**
 * @brief GPIO interrupt handler - schedule debounce worker
 *
 * This is the GPIO callback invoked by Zephyr when the motion sensor
 * GPIO asserts. It schedules a delayed work item (`motion_work`) which
 * invokes the user-provided callback after a short debounce interval.
 *
 * @param dev GPIO device pointer
 * @param cb Pointer to the gpio_callback structure
 * @param pins Bitmask of pins that triggered the callback
 */
static void motion_sensed(const struct device * dev,
                          struct gpio_callback * cb,
                          uint32_t pins)
{
  /* LOG_DBG("motion_sensor pressed pin 0x%x", pins); */
  /* Debounce: schedule the work to run after 15 ms */
  k_work_reschedule(&motion_work, K_MSEC(15));
}


/**
 * @brief Initialize motion sensor
 *
 * Configures the motion sensor GPIO and interrupt, and registers the
 * internal callback which will schedule the debounce worker. The
 * optional `handler` is invoked after the debounce period when motion
 * is confirmed.
 *
 * @param handler Optional user callback for motion events (may be NULL)
 * @return 0 on success, negative errno on failure
 */
int motion_sensor_init(motion_sensor_handler_t handler)
{
  int rc = 0;
  LOG_INF("configure motion sensor");
#ifdef CONFIG_MOTION_SENSOR_DEBOUNCE
#endif /* CONFIG_MOTION_SENSOR_DEBOUNCE */
  motion_callback = handler;
  do {
    if (!device_is_ready(motion_sensor.port)) {
      LOG_ERR("Motion sensor device not ready");
      rc = -ENODEV;
      break;
    }
    if (0 != gpio_pin_configure_dt(&motion_sensor, GPIO_INPUT)) {
      LOG_ERR("Motion sensor failed to configure");
      rc = -EIO;
      break;
    }
    if (0 != gpio_pin_interrupt_configure_dt(&motion_sensor, GPIO_INT_EDGE_TO_ACTIVE)) {
      LOG_ERR("Motion sensor failed to configure interrupts");
      rc = -EIO;
    }
    gpio_init_callback(&motion_cb_data, &motion_sensed, BIT(motion_sensor.pin));
    gpio_add_callback(motion_sensor.port, &motion_cb_data);

    LOG_INF("motion sensor inited");
  } while (0);

  return rc;
}
/**
 * @brief Set or clear the motion sensor callback
 *
 * Update the user callback invoked when motion is detected. Passing
 * `NULL` disables the callback.
 *
 * Note: the function name in the header is `set_motion_sensor_callback`;
 * this implementation keeps the original (misspelled) symbol for
 * compatibility with existing callers.
 *
 * @param handler Callback function or NULL
 */
void
set_moition_sensor_callback(motion_sensor_handler_t handler)
{
  motion_callback = handler;
}
#endif // CONFIG_USE_MOTION_SENSOR
