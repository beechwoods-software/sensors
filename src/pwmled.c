/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */

#ifdef CONFIG_USE_PWM_LED

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#
/**
 * @file pwmled.c
 * @brief PWM LED implementation
 *
 * Implements PWM-driven LED control: period/pulse configuration and
 * convenience helpers exposed by `pwmled.h`.
 */
#include <zephyr/drivers/pwm.h>

#include <zephyr/devicetree_generated.h>
#include "pwmled.h"

#include <zephyr/logging/log.h>
#include "sensors_logging.h"

LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

/*
 *
 * / {
 *	 pwms {
 *		compatible = "pwm-leds";
 *		pwmleds: pwmleds {
 *			pwms =  <&ledc0 0 PWM_HZ(100) PWM_POLARITY_NORMAL>,
 *			        <&ledc0 1 PWM_HZ(100) PWM_POLARITY_NORMAL>,
 *			        <&ledc0 2 PWM_HZ(100) PWM_POLARITY_NORMAL>;
 *		};
 *	 };
 * };
 *
 * &pinctrl {
 *   ledc0_default: ledc0_default {
 *     group1 {
 *	 pinmux = <LEDC_CH0_GPIO13>,
 *		  <LEDC_CH1_GPIO12>,
 *		  <LEDC_CH2_GPIO14>;
 *	 output-enable;
 *     };
 *   };
 * };
 * &ledc0 {
 *   pinctrl-0 = <&ledc0_default>;
 *   pinctrl-names = "default";
 *   status = "okay";
 *   #address-cells = <1>;
 *   #size-cells = <0>;
 *   channle0@0 {
 *     reg = <0x0>;
 *     timer = <0>;
 *   };
 *   channle0@1 {
 *     reg = <0x1>;
 *     timer = <0>;
 *   };
 *   channle0@2 {
 *     reg = <0x2>;
 *     timer = <0>;
 *   };
 * };
 *
 * The pwm LED is enabbled by Kconfig. The application Kconfig should have the following entry:
 * config USE_PWM_LED
 *    bool "Use PWM leds"
 *    default n
 *    help
 *      Use PWM rgb LEDS
 *
 * To enable PWM LEDS the application prj.conf or boards/<board>.conf should have the following line:
 * CONFIG_USE_PWM_LED=y
 */

#define ZEPHYR_USER zephyr_user
#define DT_DRV_COMPAT pwmleds
#define ZEPHYR_USER_NODE DT_PATH(ZEPHYR_USER)
#define LED_PWM_NODE_ID	 DT_PROP(ZEPHYR_USER_NODE,pwmleds)
#define LED_PWM_DEVICE_ID DT_PARENT(LED_PWM_NODE_ID)
//#define PWMLED_DT_SPEC(node_id) PWM_DT_SPEC_GET(node_id),
/*
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) ||        \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), pwmleds)
#error "No suitable devicetree overlay specified for pwmleds"
#endif
*/

#define PWMLED_DT_SPEC_GET_BY_IDX(node_id, prop, idx) PWM_DT_SPEC_GET_BY_IDX(node_id, idx)

static const struct pwm_dt_spec pwmleds[] = {
  DT_FOREACH_PROP_ELEM_SEP(LED_PWM_NODE_ID, pwms, PWMLED_DT_SPEC_GET_BY_IDX, (,))
};
//  DT_FOREACH_PROP_ELEM_SEP(DT_NODELABEL(pwmleds), pwms, PWMLED_DT_SPEC_GET_BY_IDX, (,))

//GPIO_DT_SPEC_GET_BY_IDX

#define NUMLEDS (sizeof(pwmleds)/sizeof(struct pwm_dt_spec))
#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)
typedef struct _pwmled_properties {
  uint32_t pulse;
  uint32_t period;
  uint32_t max_period;
}pwmled_properties_t;

pwmled_properties_t pwmled_properties[NUMLEDS];

static inline bool
pwmled_valid_led(int led_num)
{
  bool rc = false;
  if((led_num >= 0)  && (led_num < NUMLEDS)) {
    rc = true;
  }
  return rc;
}


int
pwmled_get_num_leds()
{
  return NUMLEDS;
}

static uint32_t _pwmled_get_max_period(int led_num)
{
  uint32_t max_period = 0;
  if(pwmled_valid_led(led_num)) {
    /*
     * In case the default MAX_PERIOD value cannot be set for
     * some PWM hardware, decrease its value until it can.
     *
     * Keep its value at least MIN_PERIOD * 4 to make sure
     * the sample changes frequency at least once.
     */
    LOG_DBG("Calibrating for channel %d...", pwmleds[led_num].channel);
    max_period = MAX_PERIOD;
    while (pwm_set_dt(&pwmleds[led_num], max_period, max_period / 2U)) {
      max_period /= 2U;
      if (max_period < (4U * MIN_PERIOD)) {
        LOG_ERR("Error: PWM device does not support a period at least %lu",
                4U * MIN_PERIOD);
        return 0;
      }
    }
    
    LOG_DBG("Done calibrating; maximum/minimum periods %u/%lu nsec", max_period, MIN_PERIOD);
  }
  return max_period;
}

int
pwmled_init()
{
  int num_leds = NUMLEDS;
  int rc = 0;
  int i;
  
  LOG_DBG("configure %d pwmleds", num_leds);

  for(i = 0; i < num_leds; i++) {
    LOG_INF("Configuring pwmled %d on channel %d", i, pwmleds[i].channel); 
    if(!device_is_ready(pwmleds[i].dev)) {
      LOG_ERR("pwm led %d is not ready", i);
      rc =  -1;
    }
    pwmled_properties[i].period = pwmleds[i].period;
    pwmled_properties[i].max_period = _pwmled_get_max_period(i);
    pwmled_properties[i].pulse = 0;
    
  }
  return rc;
}


int
pwmled_post()
{

  return 0;
}



int
pwmled_set_pulse(int led_num, uint32_t pulse)
{
  const struct pwm_dt_spec *ps;
  int rc = -1;
  if(pwmled_valid_led(led_num)) {
    ps = &pwmleds[led_num];
    pwmled_properties[led_num].pulse = pulse;
    rc = pwm_set(ps->dev, ps->channel,  pwmled_properties[led_num].period, pulse, ps->flags);
  }
  return rc;
}

int
pwmled_set_period(int led_num, uint32_t period)
{
  const struct pwm_dt_spec *ps;
  int rc = -1;
  if(pwmled_valid_led(led_num)) {
    ps = &pwmleds[led_num];
    if(period < pwmled_properties[led_num].max_period) {
      period = pwmled_properties[led_num].max_period;
    }
    pwmled_properties[led_num].period = period;
    rc = pwm_set(ps->dev, ps->channel,  period, pwmled_properties[led_num].pulse, ps->flags);
 }
  return rc;
}

int pwmled_set(int led_num, uint32_t period, uint32_t pulse)
{
  const struct pwm_dt_spec *ps;
  int rc = -1;
  if(pwmled_valid_led(led_num)) {
    ps = &pwmleds[led_num];
    if(period < pwmled_properties[led_num].max_period) {
      period = pwmled_properties[led_num].max_period;
    }
    pwmled_properties[led_num].pulse = pulse;
    LOG_DBG("Setting pwm %d period %d pulse %d flags 0x%x", led_num, period, pulse, ps->flags);
    rc = pwm_set(ps->dev, ps->channel,  period, pulse, ps->flags);
 }
  return rc;
}
    

uint32_t
pwmled_get_pulse(int led_num)
{
  uint32_t rc = 0;
  if(pwmled_valid_led(led_num)) {
    rc = pwmled_properties[led_num].pulse;
  }
  return rc;
}
uint32_t
pwmled_get_period(int led_num)
{
  uint32_t rc = 0;
  if(pwmled_valid_led(led_num)) {
    rc = pwmled_properties[led_num].period;
  }
  return rc;
}
uint32_t
pwmled_get_max_period(int led_num)
{
  uint32_t rc = 0;
  if(pwmled_valid_led(led_num)) {
    rc = pwmled_properties[led_num].max_period;
  }
  return rc;
}
  


static int mRGmax;
/*
 *@brief
 * set the maximum value used in Red/Green operations
 *
 *@param maximum The maximum value for red/green operations
 */
void
pwmled_set_rg_max(int maximum)
{
  LOG_WRN("pwmled_set_rg_max deprecated");
  mRGmax = maximum;
  LOG_DBG("mRGmax %d", mRGmax);
}


int
pwmled_set_leds( int moisture_level)
{
  int rc;
  int red_level;
  int green_level;
  rc = 0;
  LOG_WRN("pwmled_set_leds deprecated");
  red_level = moisture_level;
  green_level = mRGmax -  red_level ;
  
  LOG_DBG("level %d red %d green %d", moisture_level, red_level, green_level);
#if 0
  rc = pwm_set_pulse_dt(&red_pwm_led, red_level);
  if(rc) { 
    LOG_ERR("setting red pwmled failed %d",rc);
  } 
  rc = pwm_set_pulse_dt(&green_pwm_led, green_level);
  if(rc) {
    LOG_ERR("setting green pwm led failed %d",rc);
  }
  rc = pwm_set_pulse_dt(&blue_pwm_led, red_level);
  if(rc) {
    LOG_ERR("setting blue pwm led failed %d",rc);
  }
#endif
  return rc;
}



#endif // CONFIG_USE_PWM_LED
