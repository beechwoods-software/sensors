/*
 * Copyright (c) 2024
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_NFC_ST25DVXXK_H_
#define ZEPHYR_DRIVERS_NFC_ST25DVXXK_H_

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>


/* ST25DVXXK register addresses */
#define ST25DVXXK_REG_GPO             0x00
#define ST25DVXXK_REG_IT_TIME         0x01
#define ST25DVXXK_REG_EH_MODE         0x02
#define ST25DVXXK_REG_RF_MNGT         0x03
#define ST25DVXXK_REG_RFA1SS          0x04
#define ST25DVXXK_REG_ENDA1           0x05
#define ST25DVXXK_REG_RFA2SS          0x06
#define ST25DVXXK_REG_ENDA2           0x07
#define ST25DVXXK_REG_RFA3SS          0x08
#define ST25DVXXK_REG_ENDA3           0x09
#define ST25DVXXK_REG_RFA4SS          0x0A
#define ST25DVXXK_REG_I2CSS           0x0B
#define ST25DVXXK_REG_LOCK_CCFILE     0x0C
#define ST25DVXXK_REG_MB_MODE         0x0D
#define ST25DVXXK_REG_MB_WDG          0x0E
#define ST25DVXXK_REG_LOCK_CFG        0x0F
#define ST25DVXXK_REG_LOCK_DSFID      0x10
#define ST25DVXXK_REG_LOCK_AFI        0x11
#define ST25DVXXK_REG_DSFID           0x12
#define ST25DVXXK_REG_AFI             0x13
#define ST25DVXXK_REG_MEM_SIZE        0x14
#define ST25DVXXK_REG_MEM_SIZE_MSB    0x14
#define ST25DVXXK_REG_MEM_SIZE_LSB    0x15
#define ST25DVXXK_REG_BLK_SIZE        0x16
#define ST25DVXXK_REG_IC_REF          0x17
#define ST25DVXXK_REG_UID             0x18
#define ST25DVXXK_REG_IC_REV          0x20
  
#define ST25DVXXK_REG_I2C_PWD         0x0900

#define ST25DVXXK_DYN_REG_GPO_CTRL    0x2000
#define ST25DVXXK_DYN_REG_EH_CTRL     0x2002
#define ST25DVXXK_DYN_REG_RF_MNGT     0x2003
#define ST25DVXXK_DYN_REG_I2C_SSO     0x2004
#define ST25DVXXK_DYN_REG_IT_STS      0x2005
#define ST25DVXXK_DYN_REG_MB_CTRL     0x2006
#define ST25DVXXK_DYN_REG_MB_LEN      0x2007


// GPO Control for dynamic and system registers ST25DVXXK_DYN_REG_GPO_CTRL ST25DVXXK_REG_GPO
#define ST25DT64K_GPO_RF_USER          BIT(0)
#define ST25DT64K_GPO_RF_ACTIVITY      BIT(1)
#define ST25DT64K_GPO_RF_INTERRUPT     BIT(2)
#define ST25DT64K_GPO_FIELD_CHANGE     BIT(3)
#define ST25DT64K_GPO_RF_PUT_MSG       BIT(4)
#define ST25DT64K_GPO_RF_GET_MSG       BIT(5)
#define ST25DT64K_GPO_RF_WRITE         BIT(6)
#define ST25DT64K_GPO_GPO              BIT(7)

// Energy havesting system register ST25DVXXK_REG_EH_MODE
#define ST25DVXXK_EH_MODE              BIT(0)

// Energy harvesting Dynamic Register ST25DVXXK_DYN_REG_EH_CTRL
#define ST25DVXXK_EH_EH_EN             BIT(0)
#define ST25DVXXK_EH_EH_ON             BIT(1)
#define ST25DVXXK_EN_FIELD_ON          BIT(2)
#define ST25DVXXK_EN_VCC_ON            BIT(3)

// RF Management dynamic and system registers ST25DVXXK_REG_RF_MNGT
#define ST25DVXXK_RF_DISABLE           BIT(0)
#define ST25DVXXK_RF_SLEEP             BIT(1)
typedef enum _rf_mngt_state {
  RF_ENABLE = 0,
  RF_DISABLE,
  RF_SLEEP
} rf_mngt_state_t;

// Security Session dynamic register ST25DVXXK_DYN_REG_I2C_SSO
#define ST25DVXXK_SSO_OPEN             BIT(0)

// Interrupt Status dynamic register ST25DVXXK_DYN_REG_IT_STS
#define ST25DVXXK_IT_STS_RF_USER       BIT(0)
#define ST25DVXXK_IT_STS_RF_ACTIVITY   BIT(1)
#define ST25DVXXK_IT_STS_RF_INTERRUPT  BIT(2)
#define ST25DVXXK_IT_STS_FIELD_FALLING BIT(3)
#define ST25DVXXK_IT_STS_FIELD_RISING  BIT(4)
#define ST25DVXXK_IT_STS_RF_PUT_MSG    BIT(5)
#define ST25DVXXK_IT_STS_RF_GET_MSG    BIT(6)
#define ST25DVXXK_IT_STS_RF_WRITE      BIT(7)



// Fast Transfer Mode system register ST25DVXXK_REG_MB_MODE
#define ST25DT64K_MB_MODE              BIT(0)
// Fast Transfer Watchdog system register ST25DVXXK_REG_MB_WDG
#define ST25DT64K_MB_WDG_MASK          0x07

// Fast Transfer Mode Control dynamic register  ST25DVXXK_DYN_REG_MB_CTRL
#define ST25DT64K_MB_CTL_MB            BIT(0)
#define ST25DT64K_MB_HOST_PUT_MSG      BIT(1)
#define ST25DT64K_MB_RF_PUT_MSG        BIT(2)
#define ST25DT64K_MB_HOST_MISS_MSG     BIT(4)
#define ST25DT64K_MB_RF_MISS_MSG       BIT(5)
#define ST25DT64K_HOST_CURRENT_MSG     BIT(6)
#define ST25DT64K_RF_CURRNET_MSG       BIT(7)


#define ST25DT64K_B

#define ST25DVXXK_I2C_SSO_OPEN       BIT(0)

#define ST25DVXXK_SYSTEM_ADDRESS     0x04
struct st25dvxxk_config {
  struct i2c_dt_spec i2c;
  struct gpio_dt_spec int_gpio;
  //    uint8_t i2c_address;
  bool enable_ftm;
  bool enable_eh;
  bool enable_rf;
};

struct st25dvxxk_data {
    struct gpio_callback int_callback;
    struct k_work work;
    const struct device *dev;
    void (*user_callback)(const struct device *dev, uint8_t it_status);
};

#define ST25DVXXK_PASSWORD_LEN 8
typedef struct st25dvxxk_password {
  uint8_t password[ST25DVXXK_PASSWORD_LEN];
} st25dvxxk_password_t;

typedef enum security_session_type {
  PRESENT,
  WRITE
} st25dvxxk_sso_type_t;
int st25dvxxk_open_security_session(const struct device * dev, uint8_t *sso);
int st25dvxxk_close_security_session(const struct device * dev);

/**
 * @brief Initialize the ST25DVXXK NFC chip
 *
 * @param dev Pointer to the device structure
 * @return 0 on success, negative error code on failure
 */
int st25dvxxk_init(const struct device *dev);
int st25dvxxk_read_register(const struct device *dev, uint16_t reg, void *value, int len);
/**
 * @brief Read data from the NFC chip
 *
 * @param dev Pointer to the device structure
 * @param addr Memory address to read from
 * @param data Buffer to store read data
 * @param len Number of bytes to read
 * @return 0 on success, negative error code on failure
 */
int st25dvxxk_read(const struct device *dev, uint16_t addr, uint8_t *data, size_t len);

/**
 * @brief Write data to the NFC chip
 *
 * @param dev Pointer to the device structure
 * @param addr Memory address to write to
 * @param data Data to write
 * @param len Number of bytes to write
 * @return 0 on success, negative error code on failure
 */
int st25dvxxk_write(const struct device *dev, uint16_t addr, const uint8_t *data, size_t len);

/**
 * @brief Read the ST25DVXXK id
 * @param dev Pointer to the device structure
 * @param value Pionter to uint8_t buffer to store ID
 * @return 0 on success, negative error code on failuer
 */
#define st25dvxxk_read_id(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_REG_IC_REF, value, sizeof(value))

#define st25dvxxk_get_gpo(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_REG_GPO, value, sizeof(value))
#define st25dvxxk_get_gpo_ctrl(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_DYN_REG_GPO_CTRL, value, sizeof(value))



/**
 * @brief Read the memory size of the ST25DVXXK
 * @param dev Pointer to the device structure
 * @param value Pionter to uint8_t buffer to store ID
 * @return 0 on success, negative error code on failuer
 */
//int st25dvxxk_get_mem_size(const struct device *dev, uint16_t * value);
#define st25dvxxk_get_mem_size(dev, value) st25dvxxk_read_register(dev,  ST25DVXXK_REG_MEM_SIZE, value, sizeof(value))

/**
 * @brief Read the block size of the ST25DVXXK
 * @param dev Pointer to the device structure
 * @param value Pionter to uint8_t buffer to store ID
 * @return 0 on success, negative error code on failuer
 */
//int st25dvxxk_get_blk_size(const struct device *dev, uint8_t * value);
#define st25dvxxk_get_blk_size(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_REG_BLK_SIZE, value, sizeof(value))
/**
 * @brief Enable/disable Fast Transfer Mode
 *
 * @param dev Pointer to the device structure
 * @param enable true to enable, false to disable
 * @return 0 on success, negative error code on failure
 */
int st25dvxxk_set_ftm(const struct device *dev, bool enable);

/**
 * @brief Get Fast Transfer Mode
 *
 * @param dev Pointer to the device structure
 * @param value Pointer to location to save FTM mode
 * @return 0 onsuccess, negative error code on failure
 **/
//int st25dvxxk_get_ftm(const struct device * dev, uint8_t * value)
#define st25dvxxk_get_ftm(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_REG_MB_MODE, value, 1)
#define st25dvxxk_get_ftm_dyn(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_DYN_REG_MB_CTRL, value, 1)

/**
 * @brief Get the I2C SSO register
 * @param dev Pointer to the device structure
 * @param value Pointer to location to save the SSO register
 * @return 0 onsuccess, negative error code on failure
 **/
//int st25dvxxk_get_i2c_sso(const struct device * dev, uint8_t * value);
#define st25dvxxk_get_i2c_sso(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_DYN_REG_I2C_SSO, value, 1)
    
/**
 * @brief Get the fast transfer message length
 * @param dev Pointer to the device structure
 * @param value Pointer to location to save the length
 * @return 0 onsuccess, negative error code on failure
 **/
int st25dvxxk_get_mb_len(const struct device * dev, uint8_t * value);
    
/**
 * @brief Configure energy harvesting
 *
 * @param dev Pointer to the device structure
 * @param enable true to enable, false to disable
 * @param vout_3v3 true for 3.3V output, false for 5V output
 * @return 0 on success, negative error code on failure
 */
int st25dvxxk_set_energy_harvesting(const struct device *dev, bool enable);
    
/**
 * @brief Get the energy harvesting 
 * @param dev Pointer to the device structure
 * @param value Pointer to location to save the length
 * @return 0 onsuccess, negative error code on failure
 **/
#define st25dvxxk_get_energy_harvesting_mode(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_REG_EH_MODE, value, 1)
#define st25dvxxk_get_energy_harvesting_ctrl(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_DYN_REG_EH_CTRL, value, 1)


int st25dvxxk_set_rf_mngt(const struct device * dev, rf_mngt_state_t state);

/**
 * @brief Register a callback for interrupt events
 *
 * @param dev Pointer to the device structure
 * @param callback Function to call when an interrupt occurs
 * @return 0 on success, negative error code on failure
 */
int st25dvxxk_register_callback(const struct device *dev,
                               void (*callback)(const struct device *dev, uint8_t it_status));
int st25dvxxk_present_password(const struct device * dev, const st25dvxxk_password_t *password);
int st25dvxxk_write_register(const struct device *dev, uint16_t reg, uint8_t * value, int len);

int st25dvxxk_set_rf_mngt(const struct device *dev, rf_mngt_state_t state);

#define st25dvxxk_get_rf_mngt(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_REG_RF_MNGT, value, 1)
#define st25dvxxk_get_rf_mngt_dyn(dev, value) st25dvxxk_read_register(dev, ST25DVXXK_DYN_REG_RF_MNGT, value, 1)

#endif /* ZEPHYR_DRIVERS_NFC_ST25DVXXK_H_ */ 
