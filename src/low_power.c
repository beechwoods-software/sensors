/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */

#ifdef CONFIG_USE_LOWPOWER

#include <zephyr/kernel.h>
#/**
 * @file low_power.c
 * @brief Low power/sleep helper implementation
 *
 * Implements the small API used to configure and enter light or deep
 * sleep modes on supported boards.
 */

#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/gpio.h>

#ifdef CONFIG_HAS_ESP_SLEEP
#include "esp_sleep.h"
#endif // CONFIG_HAS_ESP_SLEEP

#include "low_power.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE( SENSORS_MODULE_NAME );

/* Most development boards have "boot" button attached to GPIO0.
 * You can also change this to another pin.
 */
#define SW0_NODE	DT_ALIAS(sw0)

#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "unsupported board: sw0 devicetree alias is not defined"
#endif // !DT_NODE_HAS_STATUS(SW0_NODE, okay)

/* Add an extra delay when sleeping to make sure that light sleep
 * is chosen.
 */
#define LIGHT_SLP_EXTRA_DELAY	(50UL)

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

int
low_power_init()
{
  
  LOG_DBG("Low power init");
  if (!device_is_ready(button.port)) {
    LOG_ERR("Error: button device %s is not ready", button.port->name);
    return -1;
  }
  
  const int wakeup_level = (button.dt_flags & GPIO_ACTIVE_LOW) ? 0 : 1;

#ifdef CONFIG_HAS_ESP_SLEEP
  esp_gpio_wakeup_enable(button.pin,
			 wakeup_level == 0 ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();
#else
#error "low power not supported for board"
#endif // CONFIG_HAS_ESP_SLEEP
  
  /* Wait until GPIO goes high */
  if (gpio_pin_get_dt(&button) == wakeup_level) {
    LOG_INF("Waiting for GPIO%d to go high...", button.pin);
    do {
      k_busy_wait(10000);
    } while (gpio_pin_get_dt(&button) == wakeup_level);
  }

  return 0;
}

int
low_power_set(sleep_type_t sleep_type)
{
  if(sleep_type == DEEP_SLEEP) {
    LOG_ERR("Deep sleep not supported");
  }
  
  
  LOG_INF("Entering light sleep");
  /* To make sure the complete line is printed before entering sleep mode,
   * need to wait until UART TX FIFO is empty
   */
  k_busy_wait(10000);
  
  /* Get timestamp before entering sleep */
  int64_t t_before_ms = k_uptime_get();
  
  /* Sleep triggers the idle thread, which makes the pm subsystem select some
   * pre-defined power state. Light sleep is used here because there is enough
   * time to consider it, energy-wise, worthy.
   */
  k_sleep(K_USEC(DT_PROP(DT_NODELABEL(light_sleep), min_residency_us) +
		 LIGHT_SLP_EXTRA_DELAY));
  /* Execution continues here after wakeup */
  
  /* Get timestamp after waking up from sleep */
  int64_t t_after_ms = k_uptime_get();
  
  /* Determine wake up reason */
  const char *wakeup_reason;

#ifdef CONFIG_HAS_ESP_SLEEP
  switch (esp_sleep_get_wakeup_cause()) {
  case ESP_SLEEP_WAKEUP_TIMER:
    wakeup_reason = "timer";
    break;
  case ESP_SLEEP_WAKEUP_GPIO:
    wakeup_reason = "pin";
    break;
  default:
    wakeup_reason = "other";
    break;
  }

#else
#error "low power not supported for board"
#endif // CONFIG_HAS_ESP_SLEEP
  
  LOG_INF("Returned from light sleep, reason: %s, t=%lld ms, slept for %lld ms",
				wakeup_reason, t_after_ms, (t_after_ms - t_before_ms));

  return 0;

}
#endif // CONFIG_USE_LOWPOWER
