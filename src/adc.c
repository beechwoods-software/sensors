/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */
#
/**
 * @file adc.c
 * @brief ADC sensor implementation
 *
 * Implementation of ADC sensor initialization, sampling and optional
 * debounce handling for configured ADC channels.
 */

#ifdef CONFIG_USE_ADC

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>

#include "sensors_logging.h"
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

/* 
 * The analog to digtial converter is a sensor that converts the analog voltage on a pin
 * to a digital number
 * To configure the adc sensor, the boards/<board>.overlay file should have something similar to this:
 * #include <zephyr/dt-bindings/pinctrl/esp-pinctrl-common.h>
 * #include <dt-bindings/pinctrl/esp32-pinctrl.h>
 * #include <zephyr/dt-bindings/pinctrl/esp32-gpio-sigmap.h>
 * &adc0 {
 *     status = "okay";
 * };
 * / {
 *  zephyr,user {
 *    io-channels = <&adc0 0>, <&adc0 3>, <&adc0 6>, <&adc0 7>;
 *  };
 * };
 * &adc0 {
 *  #address-cells = <1>;
 *  #size-cells = <0>;
 *
 *  channel@0 {
 *    reg = <0>;
 *    zephyr,gain = "ADC_GAIN_1";
 *    zephyr,reference = "ADC_REF_INTERNAL";
 *    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
 *    zephyr,resolution = <12>;
 *  };
 *  channel@3 {
 *    reg = <3>;
 *    zephyr,gain = "ADC_GAIN_1";
 *    zephyr,reference = "ADC_REF_INTERNAL";
 *    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
 *    zephyr,resolution = <12>;
 *  };
 *  channel@6 {
 *    reg = <6>;
 *    zephyr,gain = "ADC_GAIN_1";
 *    zephyr,reference = "ADC_REF_INTERNAL";
 *    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
 *    zephyr,resolution = <12>;
 *  };
 *  channel@7 {
 *    reg = <7>;
 *    zephyr,gain = "ADC_GAIN_1";
 *    zephyr,reference = "ADC_REF_INTERNAL";
 *    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
 *    zephyr,resolution = <12>;
 *  };
 * };
 * zephyr,user {
 *   io-channels = <&adc0 0 &adc0 3 &adc0 6 &adc0 7>;
 * };
 *
 * ADC is enabled via Kconfig.  The application Kconfig file must contain this: 
 * config USE_ADC
 *      bool "configure adc gpio ports"
 *      default n
 *      help
 *         Enable the adc gpio ports
 *
 * The applications prj.conf or boards/<board>.conf file should have these configurations
 * CONFIG_ADC=y
 * CONFIG_USE_ADC=y
 */


#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified adc io_channels"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};
#define NUM_ADC_SENSORS ARRAY_SIZE(adc_channels)
#ifdef CONFIG_USE_ADC_DEBOUNCE
/***
 * @brief adc_debounce structure
 * There is one adc_debounce structure per adc.
 * The ring buffer size is confurable.
 ***/
typedef struct adc_debounce {
  int sample[CONFIG_ADC_DEBOUNCE_SAMPLE_SIZE];
  int position;
  struct k_mutex count_mutex;
} adc_debounce_t;

adc_debounce_t adc_ring_buffer[NUM_ADC_SENSORS];

static struct k_thread debounce_thread[NUM_ADC_SENSORS];

K_THREAD_STACK_ARRAY_DEFINE(debounce_stacks, NUM_ADC_SENSORS, CONFIG_ADC_DEBOUNCE_STACK_SIZE);
void debounce_worker(void *p1, void *p2, void *p3);

bool mDone = false;
#endif // CONFIG_USE_ADC_DEBOUNCE
/*
 * @brief
 * Returns the number of configured sensors. Sensors are configured in the device tree using
 * the boards/<board>.overlay
 * @return number of configure adc ports
 */
int
bws_adc_get_num_sensors()
{
  return NUM_ADC_SENSORS;
}

/*
 * @brief 
 *  Initialize the adc ports. If a port initialization fails no further ports are configured
 *
 * @return 0 on success, -1 or  the negative error number on failure 
 */
int
bws_adc_init()
{
  int err;
#ifdef CONFIG_USE_ADC_DEBOUNCE
  char buf[8];
  k_tid_t tid;
#endif // CONFIG_USE_ADC_DEBOUNCE
  /* Configure channels individually prior to sampling. */
  for (size_t i = 0U; i < NUM_ADC_SENSORS; i++) {
    LOG_DBG("adc - %s, channel %d: ",
            adc_channels[i].dev->name,
            adc_channels[i].channel_id);
    if (!device_is_ready(adc_channels[i].dev)) {
      LOG_ERR("ADC controller device %s not ready\n", adc_channels[i].dev->name);
      return -1;
    }
    
    err = adc_channel_setup_dt(&adc_channels[i]);
    if (err < 0) {
      LOG_ERR("Could not setup channel #%d (%d)\n", i, err);
      return err;
    }
#ifdef CONFIG_USE_ADC_DEBOUNCE
    k_mutex_init(&adc_ring_buffer[i].count_mutex);
    tid = k_thread_create(&debounce_thread[i], debounce_stacks[i], CONFIG_ADC_DEBOUNCE_STACK_SIZE, debounce_worker,
                    INT_TO_POINTER(i), NULL, NULL, K_PRIO_PREEMPT(10), 0,
                    K_NO_WAIT);
    snprintf(buf,sizeof(buf),"adc-%d", i);
    k_thread_name_set(tid,buf);
#endif // CONFIG_USE_ADC_DEBOUNCE
  }
  return 0;
}

void
bws_adc_fini()
{
#ifdef CONFIG_USE_ADC_DEBOUNCE
  mDone = true;
#endif
}

/*
 * @brief
 *   Retrieve the current value of the ADC port
 *
 * @param num - the port number  (zero based)
 *
 * @return if Positve the reading on the adc port if negative the negative error number
 */
int
_bws_adc_get_sensor(int num, bool debug)
{

  int err;
  int rc = 0;
  uint16_t buf;
  struct adc_sequence sequence = {
    .buffer = &buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(buf),
  };

  int32_t val_mv;
  if(debug) {
    LOG_DBG("- %s, channel %d: ",
            adc_channels[num].dev->name,
            adc_channels[num].channel_id);
  }
  (void)adc_sequence_init_dt(&adc_channels[num], &sequence);
    
  err = adc_read(adc_channels[num].dev, &sequence);
  if (err < 0) {
    LOG_ERR("Could not read (%d)\n", err);
    return err;
  }
    
  /*
   * If using differential mode, the 16 bit value
   * in the ADC sample buffer should be a signed 2's
   * complement value.
   */
  if (adc_channels[num].channel_cfg.differential) {
    val_mv = (int32_t)((int16_t)buf);
  } else {
    val_mv = (int32_t)buf;
  }
  if(debug) {
    LOG_DBG("%"PRIx32, val_mv);
  }
  err = adc_raw_to_millivolts_dt(&adc_channels[num],
				 &val_mv);
  /* conversion to mV may not be supported, skip if not */
  if (err < 0) {
    rc = err;
    LOG_ERR(" (value in mV not available)\n");
  } else {
    if(debug) {
      LOG_DBG(" = %"PRIx32" mV\n", val_mv);
    }
    rc = val_mv;    
  }
  
  return rc;
}
int
bws_adc_get_sensor(int num)
{
  int retval;
    
#ifdef CONFIG_USE_ADC_DEBOUNCE
  int i;
  retval = 0;
  k_mutex_lock(&adc_ring_buffer[num].count_mutex, K_FOREVER);

  for(i=0; i < CONFIG_ADC_DEBOUNCE_SAMPLE_SIZE; i++) {
    retval += adc_ring_buffer[num].sample[i];
  }

  k_mutex_unlock(&adc_ring_buffer[num].count_mutex);
  retval /= CONFIG_ADC_DEBOUNCE_SAMPLE_SIZE;
#else
  retval = _bws_adc_get_sensor(num, true);
#endif
  return retval;
}
#ifdef CONFIG_USE_ADC_DEBOUNCE
void debounce_worker(void *p1, void *p2, void *p3)
{
  int adc_num = (int)p1;
  adc_debounce_t *ring_buffer = &adc_ring_buffer[adc_num];
  while(!mDone) {
    k_mutex_lock(&ring_buffer->count_mutex, K_FOREVER);
    ring_buffer->sample[ring_buffer->position++] = _bws_adc_get_sensor(adc_num,false);
    if(ring_buffer->position > CONFIG_ADC_DEBOUNCE_SAMPLE_SIZE) {
      ring_buffer->position = 0;
    }
    k_mutex_unlock(&adc_ring_buffer[adc_num].count_mutex);
    k_sleep(K_MSEC(CONFIG_ADC_DEBOUNCE_CADENCE));
  }
}

void bws_adc_dump_debounce(int num)
{
  adc_debounce_t *ring_buffer = &adc_ring_buffer[num];
  if(num > (NUM_ADC_SENSORS -1)) {
    LOG_ERR("Invalid adc sensor number %d", num);
    return;
  }
  LOG_WRN("Dumping adc %d", num);
  for( int i = 0 ; i < CONFIG_ADC_DEBOUNCE_SAMPLE_SIZE; i+=5) {
    LOG_WRN("%d %d %d %d %d", ring_buffer->sample[i], ring_buffer->sample[i+1],
        ring_buffer->sample[i+2], ring_buffer->sample[i+3], ring_buffer->sample[i+4]);
  }
}

#endif // CONFIG_USE_ADC_DEBOUNCE


#endif // CONFIG_USE_ADC
