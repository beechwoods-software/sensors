/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */
#ifdef CONFIG_USE_KY40


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#
/**
 * @file ky40.c
 * @brief KY-040 rotary encoder integration
 *
 * Handles Zephyr input events from a KY-040 rotary encoder and exposes
 * a callback interface used by higher-level application code.
 */
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include "ky40.h"

#include "sensors_logging.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

/*
    dials {
        status = "okay";
        dial0:dial_0 {
            compatible = "gpio-qdec";
            gpios = <&gpio0 25 0>, <&gpio0 26 0 >;
            steps-per-period = <4>;
            zephyr,axis = <0>;
			sample-time-us = <2000>;
			idle-timeout-ms = <200>;
        };
        dial1:dial_1 {
            compatible = gpio-qdec"
            gpios = <&gpio0 16 0>, <&gpio0 17 0 >;
            steps-per-period = <4>;
            zephyr,axis = <0>;
			sample-time-us = <2000>;
			idle-timeout-ms = <200>;
        };
    };
    buttons {
        compatible = "gpio-keys";
  	    button0: button_0 {
     	    gpios = < &gpio0 23 (GPIO_PULL_UP | GPIO_ACTIVE_LOW) >;
     	    label = "button 0";
   	    };
    	button1: button_1 {
     	    gpios = < &gpio0 22 (GPIO_PULL_UP | GPIO_ACTIVE_LOW) >;
     	    label = "button 1";
     	};
    };
    zephyr,user {
      dials = <&dial0 &dial1>;
    };


The configuration file should have the following
CONFIG_INPUT=y
CONFIG_INPUT_GPIO_QDEC=y
CONFIG_USER_KY40=y
 */



#define ZEPHYR_USER zephyr_user
#define DT_DRV_COMPAT gpio_qdec

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) ||        \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), dials)
#error "No suitable devicetree overlay specified"
#endif
#define KY40_DT_SPEC_AND_COMMA(node_id, prop, idx) \
  DEVICE_DT_INST_GET(idx),

static ky40_device_t dials[] = {
  DT_FOREACH_PROP_ELEM(DT_PATH(ZEPHYR_USER), dials, KY40_DT_SPEC_AND_COMMA)
};
static ky40_event_callback dial_callback = NULL;

static void ky40_cb_handler(struct input_event *evt, int index)
{
  LOG_DBG("Got input event from %s", evt->dev->name);
  LOG_DBG("  event type 0x%x code 0x%x value %d sync 0x%x", evt->type, evt->code, evt->value, evt->sync);
  if(NULL != dial_callback) {
    (*dial_callback)(evt, index);
  }
}


#define KY40_DT_INIT(node_id, prop, index) \
  static void ky40_cb_handler_##index(struct input_event *evt, void * user_data) \
  { \
    ARG_UNUSED(user_data); \
    ky40_cb_handler(evt, index);                      \
  } \
  INPUT_CALLBACK_DEFINE(DEVICE_DT_INST_GET(index), ky40_cb_handler_##index, NULL);


DT_FOREACH_PROP_ELEM(DT_PATH(ZEPHYR_USER), dials, KY40_DT_INIT)

#define NUM_DIALS ARRAY_SIZE(dials)

// from input_gpio_qdec.c
struct gpio_qdec_data {
	const struct device *dev;
	struct k_timer sample_timer;
	uint8_t prev_step;
	int32_t acc;
	struct k_work event_work;
	struct k_work_delayable idle_work;
	struct gpio_callback gpio_cb;
	atomic_t polling;
};


/*
 * the KY-40 is a rotary encoder
 *
 */
int  ky40_init(ky40_event_callback func)
{
  for (size_t i = 0U; i < NUM_DIALS; i++) {
    LOG_INF("Found %s API %p CONFIG %p ", dials[i]->name, dials[0]->api, dials[i]->config);
    if (!device_is_ready(dials[i])) {
      LOG_ERR("%s dial  is not ready", dials[i]->name);
      return -1;
    }

  }

  dial_callback = func;
  return NUM_DIALS;
}

int ky40_get_rotation(int dev_num)
{
  if(dev_num >= NUM_DIALS) {
    LOG_ERR("Invalid dial number %d", dev_num);
    return -1;
  }
  struct gpio_qdec_data *data = dials[dev_num]->data;

  LOG_INF("Getting Rotation");
  LOG_INF("prev step 0x%x", data->prev_step);
  LOG_INF("acc %d", data->acc);

  return data->acc;
}
#endif // CONFIG_USE_KY40
