#pragma once
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define DT_DRV_COMPAT nxp_mfrc522

// Register definitions

#define mfrc522_CommandReg           0x01
#define mfrc522_ComlEnReg            0x02
#define mfrc522_DivlEnReg            0x03
#define mfrc522_ComIrqReg            0x04
#define mfrc522_DivIrqReg            0x05
#define mfrc522_ErrorReg             0x06
#define mfrc522_Status1Reg           0x07
#define mfrc522_Status2Reg           0x08
#define mfrc522_FIFODataReg          0x09
#define mfrc522_FIFOLevelReg         0x0A
#define mfrc522_WaterLevelReg        0x0B
#define mfrc522_ControlReg           0x0C
#define mfrc522_BitFramingReg        0x0D
#define mfrc522_CollReg              0x0E

#define mfrc522_ModeReg              0x11
#define mfrc522_TxModeReg            0x12
#define mfrc522_RxModeReg            0x13
#define mfrc522_TxControlReg         0x14
#define mfrc522_TxASKReg             0x15
#define mfrc522_TxSelReg             0x16
#define mfrc522_RxSelReg             0x17
#define mfrc522_RxThresholdReg       0x18
#define mfrc522_DemodReg             0x19

#define mfrc522_MfTxReg              0x1C
#define mfrc522_MfRxReg              0x1D

#define mfrc522_SerialSpeedReg       0x1F

#define mfrc522_CRCResultReg_MSB     0x21
#define mfrc522_CRCResultReg_LSB     0x22

#define mfrc522_ModWidthReg          0x24

#define mfrc522_RFCfgReg             0x26
#define mfrc522_GsNReg               0x27
#define mfrc522_CWGsPReg             0x28
#define mfrc522_ModGsPReg            0x29
#define mfrc522_TModeReg             0x2A
#define mfrc522_TPrescalarReg        0x2B
#define mfrc522_TReloadReg_MSB       0x2C
#define mfrc522_TReloadReg_LSB       0x2D
#define mfrc522_TCounterValReg_MSB   0x2E
#define mfrc522_TCounterValReg_LSB   0x2F

#define mfrc522_TestSel1Reg          0x31
#define mfrc522_TestSel2Reg          0x32
#define mfrc522_TestPinEnReg         0x33
#define mfrc522_TestPinValueReg      0x34
#define mfrc522_TestBusReg           0x35
#define mfrc522_AutoTestReg          0x36
#define mfrc522_VersionReg           0x37
#define mfrc522_AnalogTestReg        0x38
#define mfrc522_TestDAC1Reg          0x39
#define mfrc522_TestDAC2Reg          0x3A
#define mfrc522_TestADCReg           0x3B

// Bit definitintions for the CommandReg register
#define mfrc522_CommandReg_RcvOff    0x20
#define mfrc522_CommandReg_PowerDown 0x10
// Commands for the command register CommandReg
#define mfrc522_CommandReg_Idle      0x00 
#define mfrc522_CommandReg_Mem       0x01
#define mfrc522_CommandReg_GenId     0x02
#define mfrc522_CommandReg_CRC       0x03
#define mfrc522_CommandReg_Xmit      0x04
#define mfrc522_CommandReg_NoCmd     0x07
#define mfrc522_CommandReg_Rcv       0x08
#define mfrc522_CommandReg_Trans     0x0C
#define mfrc522_CommandReg_MFAuth    0x0E
#define mfrc522_CommandReg_Reset     0x0F

// Bit definitions for the ComlEnReg register
#define mfrc522_ComlEnReg_IRqinv     0x80
#define mfrc522_ComlEnReg_TxEn       0x40
#define mfrc522_ComlEnReg_RxEn       0x20
#define mfrc522_ComlEnReg_IdleEn     0x10
#define mfrc522_ComlEnReg_HiAlretEn  0x08
#define mfrc522_ComlEnReq_LoAlertEn  0x04
#define mfrc522_ComlEnReq_ErrEn      0x02
#define mfrc522_ComlEnReq_TimerEn    0x01

// Bit definitions for the DivlEnReg register
#define mfrc522_DivlEnReg_IRQPushPull 0x80
#define mfrc522_DivlEnReg_MfinActIEn  0x10
#define mfrc522_DivlEnReg_CRCIEn      0x02








// Bit definitions of the TxControlReg register
#define mfrc522_TxControlReg_InvTx2RFOn   0x80
#define mfrc522_TxControlReg_InvTx1RFOn   0x40
#define mfrc522_TxControlReg_InvTx2RFOff  0x20
#define mfrc522_TxControlReg_InvTx1RFOff  0x10
#define mfrc522_TxControlReg_Tx2CW        0x08

#define mfrc522_TxControlReg_Tx2RFEn      0x02
#define mfrc522_TxControlReg_Tx1RFEn      0x01


// Bit definitions for the RxThresholdReg register
#define mfrc522_RxThresholdReg_MinLevel   0xF0
#define mfrc522_RxThresholdReg_CollLevel  0x07
#define RXTHRESHOLD_MINLEVEL(x) ((x << 8) & mfrc522_RxThresholdReg_MinLevel)
#define RXTHRESHOLD_COLLLEVEL(x) (x & mfrc522_RxThresholdReg_CollLevel)

// Bit defineitions for the RFCfgReg register

#define mfrc522_RFCfgReg_18DB             (0x00 << 4)
#define mfrc522_RFCfgReg_23DB             (0x01 << 4)
#define mfrc522_RFCfgReg_18DB_1           (0x02 << 4)
#define mfrc522_RFCfgReg_23DB_1           (0x03 << 4)
#define mfrc522_RFCfgReg_33DB             (0x04 << 4)
#define mfrc522_RFCfgReg_38DB             (0x05 << 4)
#define mfrc522_RFCfgReg_43DB             (0x06 << 4)
#define mfrc522_RFCfgReg_48DB             (0x07 << 4)

// Bit definitions for the GsNReg register
#define mfrc522_GsNReg_CWGsN(x)           ( ( x & 0x0f) << 4)
#define mfrc522_GsNReg_ModGsN(x)          ( x & 0x0F)

// Bit definitinos for the TModeReg register
#define mfrc522_TModeReg_TAuto            0x80
#define mfrc522_TModeReg_Gated_MASK       0x60
#define mfrc522_TModeReg_Gated_NonGated   0x00
#define mfrc522_TModeReg_Gated_MFIN       0x40
#define mfrc522_TModeReg_Gated_AUX1       0x20
#define mfrc522_TModeReg_AutoRestart      0x10

#define mfrc522_TModeReg_Prescalar_Mask   0x0f
#define TMODEREG_PRESCALAR(x) ( (x >> 8) & mfrc522_TModeReg_Prescalar_Mask)

#define TPRESCALAR_PRESCALAR(x) (x & 0xff)

#define TRELOAD_RELOAD_MSB(x) ((x >> 8) & 0xff)
#define TRELOAD_RELOAD_LSB(x) (x & 0xFF)

#define MFRC522_BUS_SPI DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)
#define MFRC522_BUS_I2C DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

union  mfrc522_bus_cfg {
#ifdef MFRC522_BUS_SPI
  struct spi_dt_spec spi;
#endif // MFRC522_BUS_SPI
#ifdef MFRC522_BUS_I2C
  struct i2c_dt_spec i2c;
#endif // MFRC522_BUS_I2C
};

typedef int (*mfrc522_register_read_t)(const struct device *dev, uint8_t reg, uint8_t *value, int len);
typedef int (*mfrc522_register_write_t)(const struct device * dev, uint8_t reg, uint8_t value);
typedef int (*mfrc522_is_ready_t)(const struct device *dev);



typedef struct mfrc522_vtable {
  mfrc522_register_read_t read;
  mfrc522_register_write_t write;
  mfrc522_is_ready_t is_ready;
} mfrc522_vtable_t;

struct mfrc522_config {
  const union mfrc522_bus_cfg bus_cfg;
  const struct mfrc522_vtable *vtable;
  struct gpio_dt_spec int_gpio;
  struct gpio_dt_spec reset_gpio;
  
};

typedef void (*mfrc522_callback_t)(const struct device * dev, uint16_t status);

struct mfrc522_data {
  struct gpio_callback int_callback;
  struct k_work work;
  const struct device *dev;
  mfrc522_callback_t user_callback;
};


/**
 * @brief Register a callback for interrupt events
 *
 * @param dev Pointer to the device structure
 * @param callback Function to call when an interrupt occurs
 * @return 0 on success, negative error code on failure
 */
int mfrc522_register_callback(const struct device *dev, mfrc522_callback_t callback);

int mfrc522_read(const struct device *dev, uint8_t reg, uint8_t *value, int len);
int mfrc522_reset(const struct device * dev);
