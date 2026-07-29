
/*
 * Copyright (c) 2025 Beechwoods Software, Inc.
 *
 */

/**
 * @file mfrc522.c
 * @brief MFRC522 RFID controller driver implementation
 *
 * Low-level register access and interrupt handling for the NXP MFRC522
 * RFID/NFC front-end. This file implements the driver vtable used by
 * board-specific device instantiations.
 */

#define DT_DRV_COMPAT nxp_mfrc522


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>

#include "mfrc522.h"

#include <zephyr/logging/log.h>
#include "sensors_logging.h"
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

#define RESET_PERIOD 100
#define RESET_COUNTDOWN 10;
#define REG_BUF_IO_SIZE 2
#define RX_MINLEVEL 8
#define RX_COLLLEVEL 4

// ~2 second timer
#define PRESCALAR 3390
#define RELOAD_COUNTER 30

#ifdef MFRC522_BUS_SPI
int mfrc522_spi_read_register(const struct device *dev, uint8_t reg, uint8_t *value, int len)
{

  int rc;
  const struct mfrc522_config * config = (const struct mfrc522_config *)dev->config;
  uint8_t addr;
  const struct spi_buf tx_buf = {
    .buf = &addr,
    .len = 1
  };
  const struct spi_buf_set tx = {
    .buffers = &tx_buf,
    .count = 1
  };
  struct spi_buf rx_buf[2] = {
    { .buf = NULL, .len = 1},
    { .buf = NULL, .len = 1}
  };
  const struct spi_buf_set rx = {
    .buffers = rx_buf,
    .count = ARRAY_SIZE(rx_buf)
  };
  int i;

  for( i = 0 ; i < len; i++) {
    addr = (reg + i ) | 0x80;
    rx_buf[1].buf = &value[i];
    rx_buf[1].len = 1;  // Redundant?
    LOG_DBG("spi tx %d 0x%x rx %d", tx.count, *(uint8_t *)(tx.buffers[0].buf), rx.count);
    rc = spi_transceive_dt(&config->bus_cfg.spi, &tx, &rx);
    if(rc) {
      LOG_DBG("Spi read transcieve failed %d", rc);
      return rc;
    }
  }
  return 0;
  
}
int mfrc522_spi_write_register(const struct device * dev, uint8_t reg, uint8_t value)
{
  int rc;
  const struct mfrc522_config * config = (const struct mfrc522_config *)dev->config;
  uint8_t cmd[] = { reg & 0x7F, value };
  const struct spi_buf tx_buf = {
    .buf = cmd,
    .len = sizeof(cmd)
  };

  const struct spi_buf_set tx = {
    .buffers = &tx_buf,
    .count = 1
  };
  
  LOG_DBG("spi tx %d 0x%x 0x%x 0x%x", tx.count, *(uint8_t *)(tx.buffers[0].buf), cmd[0], cmd[1]);
  rc = spi_write_dt(&config->bus_cfg.spi , &tx);
  if(rc) {
    LOG_ERR("spi_write failed %d",rc);
  }
  return rc;

}
int mfrc522_spi_is_device_ready(const struct device * dev)
{

  const struct mfrc522_config *config = (const struct mfrc522_config *)dev->config;
  if (!device_is_ready(config->int_gpio.port)) {
    LOG_ERR("Interrupt GPIO device not ready");
    return -ENODEV;
  }
  return 0;
}  
const mfrc522_vtable_t spi_vtable  = {
  mfrc522_spi_read_register,
  mfrc522_spi_write_register,
  mfrc522_spi_is_device_ready
};
#endif // MFRC522_BUS_SPI
#ifdef MFRC522_BUS_I2C

int mfrc522_i2c_read_register(const struct device *dev, uint8_t reg, uint8_t *value, int len)
{
  int rc = 0;
  
  return rc;
}
int mfrc522_i2c_write_register(const struct device * dev, uint8_t reg, uint8_t value) {
  int rc = 0;

  return rc;
}

int mfrc522_i2c_is_device_ready(const struct device * dev)
{
  const struct mfrc522_config *config = (const struct mfrc522_config *)dev->config;
  if (!device_is_ready(config->bus_cfg.i2c.bus)) {
    LOG_ERR("I2C bus not ready");
    return -ENODEV;
  }
  return 0;
    
}
const mfrc522_vtable_t i2c_vtable =  {
  mfrc522_i2c_read_register,
  mfrc522_i2c_write_register,
  mfrc522_i2c_is_device_ready
};
#endif // MFRC522_BUS_I2C

int mfrc522_Register_ClearBits(const struct device *dev, uint8_t reg, uint8_t mask)
{
  uint8_t value = 0;
  int rc = -1;
  
  struct mfrc522_config * config = (struct mfrc522_config *)dev->config;
  const mfrc522_vtable_t * vtable = config->vtable;
  rc = vtable->read(dev, reg, &value, 1);
  if( 0 == rc) {
    rc = vtable->write(dev, reg, value & ~mask);
  }
  return rc;
}

int mfrc522_Register_SetBits(const struct device * dev, int reg, int mask)
{
  uint8_t value = 0;
  int rc ;
  struct mfrc522_config * config = (struct mfrc522_config *)dev->config;
  const mfrc522_vtable_t * vtable = config->vtable;
  rc = vtable->read(dev, reg, &value, 1);
  if ( 0 == rc) {
    rc = vtable->write(dev, reg, value | mask);
  }
  return rc;
}
int mfrc522_read(const struct device *dev, uint8_t reg, uint8_t *value, int len)
{
  struct mfrc522_config * config = (struct mfrc522_config *)dev->config;
  const mfrc522_vtable_t * vtable = config->vtable;
  return vtable->read(dev, reg, value, len);
}
/* Worker function for handling interrupts */
static void mfrc522_work_handler(struct k_work *work)
{
  int rc;
  uint8_t status[2];
  struct mfrc522_data *data = CONTAINER_OF(work, struct mfrc522_data, work);
  const struct device *dev = data->dev;
  struct mfrc522_config * config = (struct mfrc522_config *)dev->config;
  const mfrc522_vtable_t * vtable = config->vtable;

  LOG_DBG("Interrupt 0x%x", data->int_callback.pin_mask);
  // Read clears the intrrupts
  if((rc = vtable->read(dev, mfrc522_ComIrqReg, &status[0], 1)) < 0) {
    LOG_ERR("Failed to read com interrupt status %d", rc);
    return;
  }
  if((rc = vtable->read(dev, mfrc522_DivIrqReg, &status[1], 1)) < 0) {
    LOG_ERR("Failed to read com interrupt status %d", rc);
    return;
  }
  LOG_DBG("IT_STS 0x%x", *(uint16_t *)status);
  
  /* Call user callback if registered */
  if (data->user_callback) {
    data->user_callback(dev, *(uint16_t *)status);
  }
}

/* GPIO callback for interrupt pin */
static void mfrc522_gpio_callback(const struct device *dev,
                                  struct gpio_callback *cb,
                                  gpio_port_pins_t pins)
{
    struct mfrc522_data *data = CONTAINER_OF(cb, struct mfrc522_data, int_callback);

    LOG_DBG("GPIO Callback for 0x%x", pins);

    k_work_submit(&data->work);
}

int mfrc522_reset(const struct device *dev)
{
  struct mfrc522_config * config = (struct mfrc522_config *)dev->config;
  int rc = 0;
  if(config->reset_gpio.port) {
    do {
      rc = gpio_pin_get_dt(&config->reset_gpio);
      LOG_DBG("Reset status %d", rc);
      if(rc< 0) {
        LOG_ERR("Uable to read rset pin %d",rc);
        break;
      }
      rc = gpio_pin_set_dt(&config->reset_gpio, 0);
      if(rc < 0) {
        LOG_ERR("Unable to write reset pin %d", rc);
        break;
      }
      k_msleep(2);
      rc = gpio_pin_set_dt(&config->reset_gpio, 1);
      if(rc < 0) {
        LOG_ERR("Unable to re write reset pin %d", rc);
        break;
      }
      k_msleep(50);
      rc = gpio_pin_get_dt(&config->reset_gpio);
      LOG_DBG("Reset pin %d", rc);
      rc = 0;
    } while(0);
      
  };
  return rc;
}
int mfrc522_bus_init(const struct device * dev)
{
  int rc = 0;
  struct mfrc522_data *data = (struct mfrc522_data *)dev->data;
  const struct mfrc522_config *config = (const struct mfrc522_config *)dev->config;

  /* Initialize work queue */
  k_work_init(&data->work, mfrc522_work_handler);
  data->dev = dev;
  
  /* Configure interrupt GPIO if available */
  if (config->int_gpio.port) {
    LOG_DBG("mfrc522 configuring GPIO port %d", config->int_gpio.pin);
    if (!device_is_ready(config->int_gpio.port)) {
      LOG_ERR("Interrupt GPIO device not ready");
      return -ENODEV;
    }
    
    rc = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT | GPIO_INT_EDGE_FALLING);
    if (rc < 0) {
      LOG_ERR("Failed to configure interrupt pin %d", rc);
      return rc;
    }
    
    gpio_init_callback(&data->int_callback, mfrc522_gpio_callback,
                       BIT(config->int_gpio.pin));
    
    rc = gpio_add_callback(config->int_gpio.port, &data->int_callback);
    if (rc < 0) {
      LOG_ERR("Failed to add GPIO callback %d",rc);
      return rc;
    }
    
    rc = gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_FALLING);
    if (rc < 0) {
      LOG_ERR("Failed to configure interrupt %d", rc);
      return rc;
    }
  }

  if(config->reset_gpio.port) {
    LOG_INF("Configuring reset gpio on pin %d", config->reset_gpio.pin);
    if (!gpio_is_ready_dt(&config->reset_gpio)) {
      return -ENODEV;
    }
    rc = gpio_pin_configure_dt(&config->reset_gpio, GPIO_OUTPUT | GPIO_INPUT);
    if (rc < 0) {
      LOG_ERR("Could not configure reset gpio: %d", rc);
      return rc;
    }
  }
  return rc;
}

int mfrc522_chip_init(const struct device * dev)
{
  int rc;
  uint8_t value;
  int loopcount = RESET_COUNTDOWN;
  struct mfrc522_config * config = (struct mfrc522_config *)dev->config;
  const mfrc522_vtable_t * vtable = config->vtable;
  LOG_DBG("CHIP_INIT");

  mfrc522_reset(dev);
  rc = vtable->write(dev, mfrc522_CommandReg, mfrc522_CommandReg_Reset);
  if(rc ) {
    LOG_ERR("Unable to soft reset %d",rc);
    return rc;
  }
  do {
    rc = vtable->read(dev, mfrc522_CommandReg, &value, 1);
    if ((0 == rc) && !(value & mfrc522_CommandReg_PowerDown)) {
      break;
    }
    LOG_DBG("Value 0x%x", value);
    k_sleep(Z_TIMEOUT_MS(RESET_PERIOD));
  } while(--loopcount);
  if(!loopcount) {
    LOG_ERR("Init Reset failed 0x%x", value);
    return -1;
  }
  
  rc = mfrc522_Register_ClearBits(dev, mfrc522_TxControlReg, (mfrc522_TxControlReg_Tx2RFEn | mfrc522_TxControlReg_Tx1RFEn));

  k_sleep(Z_TIMEOUT_MS(1000)); // FIX THIS
  
  rc = mfrc522_Register_SetBits(dev, mfrc522_TxControlReg, (mfrc522_TxControlReg_Tx2RFEn | mfrc522_TxControlReg_Tx1RFEn));
  
  rc = vtable->write(dev, mfrc522_TModeReg, mfrc522_TModeReg_TAuto | TMODEREG_PRESCALAR(PRESCALAR));

  rc = vtable->write(dev, mfrc522_TPrescalarReg, TPRESCALAR_PRESCALAR(PRESCALAR));
                              
  rc = vtable->write(dev, mfrc522_TReloadReg_LSB, TRELOAD_RELOAD_LSB(RELOAD_COUNTER));
  rc = vtable->write(dev, mfrc522_TReloadReg_MSB, TRELOAD_RELOAD_MSB(RELOAD_COUNTER));
  // Enable interrupts
  rc = vtable->write(dev, mfrc522_DivlEnReg, mfrc522_DivlEnReg_IRQPushPull | mfrc522_DivlEnReg_MfinActIEn);

  rc = vtable->write(dev, mfrc522_RxThresholdReg, RXTHRESHOLD_MINLEVEL(RX_MINLEVEL) | RXTHRESHOLD_COLLLEVEL(RX_COLLLEVEL));

  rc = vtable->write(dev, mfrc522_RFCfgReg, mfrc522_RFCfgReg_43DB);

  rc = vtable->write(dev, mfrc522_GsNReg, mfrc522_GsNReg_CWGsN(0x0f) | mfrc522_GsNReg_ModGsN(0x0f));
  
  rc = vtable->write(dev, mfrc522_CWGsPReg, 0x2f);

  return rc;
}



int
mfrc522_init(const struct device * dev)
{
  int rc;
  LOG_DBG("mfrc522 init");
  do {
    rc = mfrc522_bus_init(dev);
    if( 0 != rc){
      LOG_ERR("mfrc522 bus init failed %d", rc);
      break;
    }
    rc = mfrc522_chip_init(dev);
    if( rc != 0) {
        LOG_ERR("mfrc522 chip init failed %d", rc);
        break;
    }
    
  } while(0);
  return rc;
}


#define MFRC522_CONFIG_SPI(inst)                                       \
    .bus_cfg.spi = SPI_DT_SPEC_INST_GET(                               \
      inst, MFRC522_SPI_OPERATION, 0),                                 \
    .vtable = &spi_vtable,                                             \
      
#define MFRC522_CONFIG_I2C(inst)                                       \
    .bus.i2c = I2C_DT_SPEC_INST_GET(inst),                             \
    .vtable = &i2c_vtable,                                             \
  
#if 0
    .cs_gpio = SPI_CS_GPIOS_DT_SPEC_INST_GET(inst),                    \
    .int_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, int_gpios, {0}),        \
    .reset_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, reset_gpuios, {0}),   \
  
#endif
#define MFRC522_SPI_OPERATION (SPI_WORD_SET(8) | SPI_TRANSFER_MSB) 
    
#define MFRC522_INIT(inst)                                             \
  static struct mfrc522_data mfrc522_data##inst;                       \
                                                                       \
  static const struct mfrc522_config mfrc522_config##inst = {          \
		COND_CODE_1(DT_INST_ON_BUS(inst, spi),			               \
			    (MFRC522_CONFIG_SPI(inst)),			                   \
			    (MFRC522_CONFIG_I2C(inst)))                            \
    .int_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, int_gpios, {0}),        \
    .reset_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, reset_gpios, {0}),    \
  };			                                                       \
                                                                       \
  DEVICE_DT_INST_DEFINE(inst,                                          \
                        mfrc522_init,                                  \
                        NULL,                                          \
                        &mfrc522_data##inst,                           \
                        &mfrc522_config##inst,                         \
                        POST_KERNEL,                                   \
                        CONFIG_MFRC522_INIT_PRIORITY,                  \
                        NULL);

DT_INST_FOREACH_STATUS_OKAY(MFRC522_INIT)
