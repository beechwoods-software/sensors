/*
 * Copyright (c) 2025 Beechwoods Software, Inc.
 *
 */

#define DT_DRV_COMPAT st_st25dvxxk

/**
 * @file st25dvxxk.c
 * @brief ST25DVXXK NFC driver implementation
 *
 * Implementation of register-level helpers and driver hooks for the
 * ST ST25DVXXK NFC tag IC.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include "st25dvxxk.h"

LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );


static st25dvxxk_password_t st25dvxxk_password = {0};
static st25dvxxk_password_t st25dvxxk_close_password = {
  { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}
};

int st25dvxxk_set_password(const struct device * dev, const st25dvxxk_password_t *password, st25dvxxk_sso_type_t type)
{
  int rc;
  uint8_t buffer[17];
  int i;
  for(i = 0 ; i < ST25DVXXK_PASSWORD_LEN; i++) {
    buffer[i] = buffer[i+9] = password->password[i];
  }
  if(type == PRESENT) {
    buffer[8] = 0x09;
  } else {
    buffer[8] = 0x07;
    // TODO disable ftm for WRITE password
  }
  rc = st25dvxxk_write_register(dev,  ST25DVXXK_REG_I2C_PWD, buffer,17);
  return rc;

}
int st25dvxxk_open_security_session(const struct device * dev, uint8_t *sso)
{
  int rc = 0;
  rc = st25dvxxk_get_i2c_sso(dev, sso);
  if( 0 == rc) {
    LOG_DBG("SS is 0x%x",*sso);
    if(!(*sso & ST25DVXXK_SSO_OPEN)) {
      LOG_DBG("OPening SSO ");
      rc = st25dvxxk_set_password(dev, &st25dvxxk_password, PRESENT);
      if( 0 != rc) {
        LOG_ERR("Unable to present password %d",rc);
        return rc;
      }
    }
  } else {
    LOG_ERR("Unable to read sso");
  }
  return rc;
}

int st25dvxxk_close_security_session(const struct device * dev)
{
  int rc;
  uint8_t sso;
  //LOG_DBG("Close SSO");
  k_sleep(K_MSEC(10));
  rc = st25dvxxk_get_i2c_sso(dev, &sso);
  if( 0 == rc) {
    LOG_DBG("SS is 0x%x",sso);
    if(sso & ST25DVXXK_SSO_OPEN) {
      LOG_DBG("CLoseing SSO ");
      rc = st25dvxxk_set_password(dev, &st25dvxxk_close_password, PRESENT);
      if( 0 != rc) {
        LOG_ERR("Close sso failed %d",rc);
      }
    }
  } else {
    LOG_ERR("Unable to read sso %d", rc);
  }
  return rc;
}
  
/* Read a i2c register */
int st25dvxxk_read_register(const struct device *dev, uint16_t reg, void *value, int len)
{
  int rc;
  uint8_t target_reg[2];
  const struct st25dvxxk_config *config = dev->config;
  struct i2c_dt_spec i2c_system = config->i2c;

  // System address are one octet, dynamic addresses start at 0x2000
  if(reg < 255) {
    i2c_system.addr |= ST25DVXXK_SYSTEM_ADDRESS;
  }

  //#define TEST_8BIT_I2C 1
#ifdef TEST_8BIT_I2C
  /* this is the way it should work */
  rc =  i2c_reg_read_byte_dt(&i2c_system, reg, value);
  LOG_DBG("Using i2c_reg_read_byte_dt value 0x%x", *(uint8_t *)value);

  uint8_t reg_8bit;
  reg_8bit = reg;
  rc = i2c_write_read(config->i2c.bus, i2c_system.addr, &reg_8bit, sizeof(reg_8bit), value, len);
  LOG_DBG("using 8 bit i2c_write_read  value 0x%x",  *(uint8_t *)value);
#endif
 
  sys_put_be16(reg, target_reg);
  rc = i2c_write_read(config->i2c.bus, i2c_system.addr, target_reg, sizeof(target_reg), value, len);

  return rc;
}

int st25dvxxk_write_register(const struct device *dev, uint16_t reg, uint8_t * value, int len) {
  int rc;
  uint8_t target[2];
  const struct st25dvxxk_config *config = dev->config;
  struct i2c_dt_spec i2c_dt = config->i2c;
  // System address are smaller than dynamic addresses which start at 0x2000
  if(reg < ST25DVXXK_DYN_REG_GPO_CTRL) {
    i2c_dt.addr |= ST25DVXXK_SYSTEM_ADDRESS;
  }
  sys_put_be16(reg, target);
  struct i2c_msg msgs[2] = {
    {.buf = target, .len = 2, .flags = I2C_MSG_WRITE},
    {.buf = value, .len = len, .flags = I2C_MSG_WRITE | I2C_MSG_STOP}
  };
  
  k_sleep(K_MSEC(1));
  rc =  i2c_transfer_dt(&i2c_dt, msgs, 2);
                                                
  if( 0 != rc) {
    LOG_ERR("write register failed %d", rc);
  }
  return rc;
}
/* Worker function for handling interrupts */
static void st25dvxxk_work_handler(struct k_work *work)
{
  int rc;
  struct st25dvxxk_data *data = CONTAINER_OF(work, struct st25dvxxk_data, work);
  const struct device *dev = data->dev;
  uint8_t it_status;

  LOG_DBG("Interrupt 0x%x", data->int_callback.pin_mask);
  // Read clears the intrrupts
  if((rc = st25dvxxk_read_register(dev, ST25DVXXK_DYN_REG_IT_STS, &it_status, 1)) < 0) {
    LOG_ERR("Failed to read interrupt status %d", rc);
    return;
  }
  LOG_DBG("IT_STS 0x%x", it_status);
  
  /* Call user callback if registered */
  if (data->user_callback) {
    data->user_callback(dev, it_status);
  }
}

/* GPIO callback for interrupt pin */
static void st25dvxxk_gpio_callback(const struct device *port,
                                  struct gpio_callback *cb,
                                  gpio_port_pins_t pins)
{
    struct st25dvxxk_data *data = CONTAINER_OF(cb, struct st25dvxxk_data, int_callback);

    LOG_DBG("GPIO Callback for 0x%x", pins);

    k_work_submit(&data->work);
}

/* Initialize the ST25DVXXK device */
int st25dvxxk_init(const struct device *dev)
{
    const struct st25dvxxk_config *config = dev->config;
    struct st25dvxxk_data *data = dev->data;
    int ret;
    LOG_DBG("st25dvxxk init");
    LOG_DBG("is device ready  %d init_res %d", config->i2c.bus->state->initialized, config->i2c.bus->state->init_res);

    if (!device_is_ready(config->i2c.bus)) {
        LOG_ERR("I2C bus not ready");
        return -ENODEV;
    }

    /* Initialize work queue item */
    k_work_init(&data->work, st25dvxxk_work_handler);
    data->dev = dev;

    LOG_DBG("Configuring GPIO port %d", config->int_gpio.pin);
    /* Configure interrupt GPIO if available */
    if (config->int_gpio.port) {
      LOG_DBG("Port OK");
        if (!device_is_ready(config->int_gpio.port)) {
            LOG_ERR("Interrupt GPIO device not ready");
            return -ENODEV;
        }

        ret = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT | GPIO_INT_EDGE_FALLING);
        if (ret < 0) {
            LOG_ERR("Failed to configure interrupt pin");
            return ret;
        }

        gpio_init_callback(&data->int_callback, st25dvxxk_gpio_callback,
                         BIT(config->int_gpio.pin));

        ret = gpio_add_callback(config->int_gpio.port, &data->int_callback);
        if (ret < 0) {
            LOG_ERR("Failed to add GPIO callback");
            return ret;
        }

        ret = gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_FALLING);
        if (ret < 0) {
            LOG_ERR("Failed to configure interrupt");
            return ret;
        }
    }

    /* Configure Fast Transfer Mode if enabled */
    if (config->enable_ftm) {
      LOG_DBG("Configure ftm");
        ret = st25dvxxk_set_ftm(dev, true);
        if (ret < 0) {
            LOG_ERR("Failed to enable Fast Transfer Mode");
            //            return ret;
        }
    }

    /* Configure Energy Harvesting if enabled */
    if (config->enable_eh) {
      LOG_DBG("Configure eh");
        ret = st25dvxxk_set_energy_harvesting(dev, true);
        if (ret < 0) {
            LOG_ERR("Failed to enable Energy Harvesting");
            //            return ret;
        }
    }
    if(config->enable_rf) {
      LOG_DBG("Enable RF");
      ret = st25dvxxk_set_rf_mngt(dev, RF_ENABLE);
      if(0 != ret) {
        LOG_ERR("Failed to enable RF %d", ret);
      }
    }

    return 0;
}

/* Read data from the eeprom of the  NFC chip */
int st25dvxxk_read(const struct device *dev, uint16_t addr,
                   uint8_t *data, size_t len)
{
  uint8_t addr_buf[2];
  const struct st25dvxxk_config *config = dev->config;
  sys_put_be16(addr, addr_buf);
  
  return i2c_write_read_dt(&config->i2c, addr_buf, sizeof(addr_buf),
                            data, len);
}

/* Write data to the eeprom of NFC chip */
int st25dvxxk_write(const struct device *dev, uint16_t addr,
                    const uint8_t * const data, size_t len)
{
    const struct st25dvxxk_config *config = dev->config;
  struct i2c_dt_spec i2c_dt = config->i2c;
    
    uint8_t addr_buf[2];
    struct i2c_msg msg[2] = {
      {.buf = addr_buf, .len = 2, .flags = I2C_MSG_WRITE},
      {.buf = (uint8_t *)data, .len = len, .flags = I2C_MSG_WRITE | I2C_MSG_STOP}
    };
        
    if (len > 256) {
        return -EINVAL;
    }
    sys_put_be16(addr, addr_buf);
    return i2c_transfer_dt(&i2c_dt, msg, 2);
}
int st25dvxxk_write_secure_register(const struct device * dev, uint16_t reg, uint8_t value)
{
  int rc;
  uint8_t sso;
  rc = st25dvxxk_open_security_session(dev, &sso);
  if( 0 == rc) {
    rc =  st25dvxxk_write_register(dev, reg, &value, 1);
    if( 0 != rc) {
      LOG_ERR("Unable to set register");
    }
    rc |= st25dvxxk_close_security_session(dev);
  }
  
  return rc;
}
/* Enable/disable Fast Transfer Mode */
int st25dvxxk_set_ftm(const struct device *dev, bool enable)
{
  int rc;
  uint8_t sso;
  uint8_t ftm_config = enable ? ST25DT64K_MB_MODE : 0;
  rc = st25dvxxk_open_security_session(dev, &sso);
  if( 0 == rc) {
    LOG_DBG("Setting MB_MODE to %d", ftm_config);
    rc =  st25dvxxk_write_register(dev, ST25DVXXK_REG_MB_MODE, &ftm_config, 1);
    if( 0 != rc) {
      LOG_ERR("Unable to set MB MODE");
    }
    //    LOG_DBG("Closing sso");
      
    rc |= st25dvxxk_close_security_session(dev);
  }
  
  return rc;
}

int st25dvxxk_get_mb_len(const struct device * dev, uint8_t *value)
{
  return st25dvxxk_read_register(dev,  ST25DVXXK_DYN_REG_MB_LEN, value, sizeof(value)); 
}

/* Configure energy harvesting */
int st25dvxxk_set_energy_harvesting(const struct device *dev, bool enable)
{
  int rc;
  uint8_t eh_config = enable? ST25DVXXK_EH_MODE:0;
  rc = st25dvxxk_write_secure_register(dev,ST25DVXXK_REG_RF_MNGT, eh_config); 
  if( 0 != rc) {
    LOG_ERR("Unable to set energy harvesting %d",rc);
  }
  return rc;
}

/* configure RF */
int st25dvxxk_set_rf_mngt(const struct device *dev, rf_mngt_state_t state)
{
  int rc;
  uint8_t rf_mgnt = 0;
  if(state == RF_DISABLE) {
    rf_mgnt = ST25DVXXK_RF_DISABLE;
  } else if (state == RF_SLEEP) {
    rf_mgnt = ST25DVXXK_RF_SLEEP;
  }
  rc = st25dvxxk_write_secure_register(dev,ST25DVXXK_REG_RF_MNGT, rf_mgnt); 
  if( 0 != rc) {
    LOG_ERR("Unable to set rf management %d",rc);
  }
  return rc;
}
  

/* Register interrupt callback */
int st25dvxxk_register_callback(const struct device *dev,
                               void (*callback)(const struct device *dev,
                                              uint8_t it_status))
{
    struct st25dvxxk_data *data = dev->data;

    data->user_callback = callback;
    return 0;
}

#define ST25DVXXK_INIT(inst)                                            \
    static struct st25dvxxk_data st25dvxxk_data_##inst;                 \
                                                                        \
    static const struct st25dvxxk_config st25dvxxk_config_##inst = {    \
        .i2c = I2C_DT_SPEC_INST_GET(inst),                              \
        .int_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, int_gpios, {0}),     \
        .enable_ftm = DT_INST_PROP(inst, enable_ftm),                   \
        .enable_eh = DT_INST_PROP(inst, enable_eh),                     \
        .enable_rf = DT_INST_PROP(inst, enable_rf),                     \
    };                                                                  \
                                                                        \
    DEVICE_DT_INST_DEFINE(inst,                                         \
                         st25dvxxk_init,                                \
                         NULL,                                          \
                         &st25dvxxk_data_##inst,                        \
                         &st25dvxxk_config_##inst,                      \
                         POST_KERNEL,                                   \
                         CONFIG_ST25DVXXK_INIT_PRIORITY,                \
                         NULL);

DT_INST_FOREACH_STATUS_OKAY(ST25DVXXK_INIT) 
