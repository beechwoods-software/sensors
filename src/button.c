/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */

#ifdef CONFIG_USE_BUTTON

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "button.h"

#include <zephyr/logging/log.h>
#include "sensors_logging.h"
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

/**
 * @file button.c
 * @details
 * The button sensor is a on/off toggle.
 * To configure the button, the boards/\<board\>.overlay file should contain  something like this:
 * @code
 * / {
 *   buttons {
 *       compatible = "gpio-keys";
 * 	button0: button_0 {
 *    	    gpios = < &gpio0 23 (GPIO_PULL_UP | GPIO_ACTIVE_LOW) >;
 *    	    label = "button 0";
 *  	};
 *  	button1: button_1 {
 *    	    gpios = < &gpio0 22 (GPIO_PULL_UP | GPIO_ACTIVE_LOW) >;
 *    	    label = "button 1";
 *  	};
 *   };
 * };
  * @endcode
* The button is enabled and configured via Kconfig. Your Kconfig file should contain something like this
 * @code
 * config USE_BUTTON
 *      bool "Use button for <purpose for the buttons> "
 *      default n
 *      help
 *        Use Button to <purpose of button>
 *
 * @endcode
 * And the applications prj.conf or boards/\<board\>.conf file should have these configurations
 *
 * CONFIG_USE_BUTTON=y
 *
 * button_init intializes the button(s). It takes a pointer to a callback structure.
 *   The callback structure contains the button number, and the state of the button indicating
 *   wether the button was pressed or released. The callback can be NULL
 * To change the callback during the program use the function set_button_callback.
 *   Again the callback handler may be NULL
 */

/** @brief where to find the alias for the button in the device tree */
#define ZEPHYR_USER zephyr_user
/** @brief The driver compatibility for device ddriver for this button */
#define DT_DRV_COMPAT gpio-keys

/**
 * @brief Create a dt spec for a button specified in the device tree
 */
#define BUTTON_DT_SPEC_GET_BY_IDX(node_id, prop, idx)   \
  { \
    .button_spec.port = DEVICE_DT_GET(DT_PHANDLE_BY_IDX(DT_GPIO_CTLR_BY_IDX(node_id, prop, idx), gpios, 0)), \
    .button_spec.pin = DT_GPIO_PIN( DT_GPIO_CTLR_BY_IDX(node_id, prop, idx ) , gpios), \
    .button_spec.dt_flags = DT_GPIO_FLAGS(DT_GPIO_CTLR_BY_IDX(node_id, prop, idx), gpios), \
  }

/**
 * @brief create an array element for each button specified in the device tree
 */
#define BUTTON_DT_SPEC_AND_COMMA(node_id, prop, idx)    \
  BUTTON_DT_SPEC_GET_BY_IDX(node_id, prop , idx),

/** @brief Structure defininging instance specific data and callbacks */
struct button_data {
  struct gpio_dt_spec button_spec;
  struct gpio_callback button_cb;
};

/**
 * @brief Instance specific data and callbacks for each button specified in the device tree
 */
static struct button_data  buttons[] = {
  DT_FOREACH_PROP_ELEM(DT_PATH(ZEPHYR_USER), buttons, BUTTON_DT_SPEC_AND_COMMA)
};

/**
 * @brief the number of buttons defined in the device tree
 */
#define NUM_BUTTONS ARRAY_SIZE(buttons)

static button_state_handler_t callback = NULL;

static void cooldown_expired(int val, int button)
{
   button_state_t state = val ? BUTTON_STATE_PRESSED : BUTTON_STATE_RELEASED;
   LOG_DBG("Button%d state: %d",button, state);
   if (callback) {
     button_callback_data_t cbdata;
     cbdata.button = button;
     cbdata.state = state;
     LOG_DBG("call user button %d state %d", button, state);
     (*callback)(&cbdata);
   } else {
     LOG_ERR("no button callback handler");
   }
}

/** @brief Macro to create instance specific data and funtions */
#define BUTTON_DT_INIT(node_id, prop, idx) \
static void cooldown_expired##idx(struct k_work *work) \
{ \
  ARG_UNUSED(work); \
  int val = gpio_pin_get_dt(&buttons[idx].button_spec);  \
  LOG_DBG("cooldown%d 0x%x",idx, val);                  \
  cooldown_expired(val, idx); \
}\
static K_WORK_DELAYABLE_DEFINE(cooldown_work##idx, cooldown_expired##idx); \
void button_pressed##idx(const struct device * dev, \
		    struct gpio_callback * cb, \
		    uint32_t pins) \
{ \
  LOG_DBG("button%d pressed", idx); \
  k_work_reschedule(&cooldown_work##idx, K_MSEC(15)); \
}

#define BUTTON_PRESSED_AND_COMMA(node_id, prop, idx) \
  button_pressed##idx ,

/** @brief initialiation for all buttons foundin zephyr_user */
DT_FOREACH_PROP_ELEM(DT_PATH(ZEPHYR_USER), buttons, BUTTON_DT_INIT)

/** @brief arrray of GPIO callback handlers for each button */
gpio_callback_handler_t button_pressed[] = {
  DT_FOREACH_PROP_ELEM(DT_PATH(ZEPHYR_USER), buttons, BUTTON_PRESSED_AND_COMMA)
};


int button_init(button_state_handler_t handler)
{
  int rc = 0;
  int i;
  LOG_DBG("Configure button");
  do {
    for(i = 0 ; (i < NUM_BUTTONS) && (rc >= 0); i++) {
      LOG_INF("Configuring button%d on gpio %d", i, buttons[i].button_spec.pin);
      if(!device_is_ready(buttons[i].button_spec.port)) {
        LOG_ERR("button%d not ready: %d", i, rc);
        rc = -1;
        break;
      }
      if(0 != gpio_pin_configure_dt(&buttons[i].button_spec, GPIO_INPUT)) {
        LOG_ERR("button%d failed configure %d", i, rc);
        rc = -1;
        break;
      }
      if(0 != gpio_pin_interrupt_configure_dt(&buttons[i].button_spec, GPIO_INT_EDGE_BOTH)) {
        LOG_ERR("button%d failed interrupt configure: %d", i, rc);
        rc = -1;
        break;
      }

      gpio_init_callback(&buttons[i].button_cb, button_pressed[i], BIT(buttons[i].button_spec.pin));
      gpio_add_callback(buttons[i].button_spec.port, &buttons[i].button_cb);
    }
    callback = handler;
    LOG_DBG("button inited");
  }while(0);

  return rc;
}

void
set_button_callback( button_state_handler_t cb)
{
  callback = cb;
}
#endif // CONFIG_USE_BUTTON
