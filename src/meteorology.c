/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */

#ifdef CONFIG_USE_METEOROLOGY_SENSORS

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#
/**
 * @file meteorology.c
 * @brief Environmental sensors (temperature/humidity/pressure)
 *
 * Provides initialization and read helpers for environmental sensors
 * used by the application.
 */
#include <zephyr/drivers/sensor.h>

/**
 * Meteorology.c - this file contains code to read data from atmospheric sensors (temperature, humidity, barametric pressure)
 **/




#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE( SENSORS_MODULE_NAME );

/*
 * This file supports the bme280, bmp280, dht11 and dht22
 *
 * This is a module for the BME280 barametric pressure sensor and the BME280 Humidity sensor
 * The BMP280 is very similar to the BME280 so the BME280 driver will work
 * The BMP280 supports both I2C and SPI 
 * To configure the sensors the boards/<board>.overlay file should contain the following:
 * For i2C
 * &i2c0 {
 *      status = "okay";
 *      pinctrl-0 = <&i2c0_default>;
 *      pinctrl-names = "default";
 *
 *      bme280@76 {
 *              compatible = "bosch,bme280";
 *              status = "okay";
 *              reg = <0x76>;
 *      };
 * };
 * For SPI
 *
 * This is a coniguration for an rpi_pico_w
 * &pinctrl {
 *	pio0_spi0_default: pio0_spi0_default {
 *		group1 {
 *			pinmux = <PIO0_P13>, <PIO0_P14>, <PIO0_P15>;
 *		};
 *		group2 {
 *			pinmux = <PIO0_P12>;
 *			input-enable;
 *		};
 *	};
 *
 * };
 *
 * &pio0 {
 *	status = "okay";
 *
 *	pio0_spi0: pio0_spi0 {
 *		pinctrl-0 = <&pio0_spi0_default>;
 *		pinctrl-names = "default";
 *
 *		compatible = "raspberrypi,pico-spi-pio";
 *		status = "okay";
 *	};
 * };
 *
 * &pio0_spi0 {
 *	status = "okay";
 *	#address-cells = <1>;
 *	#size-cells = <0>;
 *	clocks = < &system_clk >;
 *	miso-gpios = <&gpio0 12 0>;
 *	cs-gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
 *	clk-gpios = <&gpio0 14 GPIO_ACTIVE_HIGH>;
 *	mosi-gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>;
 *	bme280@0 {
 *		compatible = "bosch,bme280";
 *		reg = <0>;
 *		spi-max-frequency = <1000000>; 
 *	};
 * };
 *
 *
 *
 * To configure the sensor for the esp32 the boards/esp32.overlay should have something like :
 * &spi3 {
 *
 *	bme280@0 {
 *		compatible = "bosch,bme280";
 *		reg = <0>;
 *		spi-max-frequency = <1000000>; 
 *	};
 * };
 *
 * The DHT22 and DHT11 temperature sensors provide both humidity and temperature readings
 *
 * To configure the sensors the boards/<board>.overlay file should contain the following:
 * If the sensor is a DHT11 the dht22 line should be removed
 * / {
 *     dht22 {
 *         compatible = "aosong,dht";
 *	   status = "okay";
 *	   dio-gpios = <&gpio0 0 0>;
 *	   dht22; 
 *      };
 * };      
 *
 * The BMP388/BMP390L sensors provide pressure
 *       bmp388: bmp388@76 {
 *    	      compatible = "bosh, bmp388";
 *           reg = <0x76>
 *           int-gpios=<&gpio0 2 GPIO_ACTIVE_LOW>
 */

static const  struct device *temp_dev;
static const struct device *humidity_dev;
static const struct device *pressure_dev;

int meteorology_init()
{
  temp_dev = NULL;
  humidity_dev = NULL;
  pressure_dev = NULL;

  const struct device * dev = NULL;
#if defined(CONFIG_BME280)
  dev = DEVICE_DT_GET_ANY(bosch_bme280);
  temp_dev = dev;
  humidity_dev = dev;
  pressure_dev = dev;
#elif defined(CONFIG_DHT22)
  dev =  DEVICE_DT_GET_ONE(aosong_dht);
  temp_dev = dev;
  humidity_dev = dev;
#elif defined(CONFIG_BMP388)
  dev = DEVICE_DT_GET_ANY(bosch_bmp388);
  temp_dev = dev;
  humidity_dev = dev;
  pressure_dev = dev;
#elif defined (CONFIG_SHT4X)
  dev = DEVICE_DT_GET_ANY(sensirion_sht4x);
  temp_dev = dev;
  humidity_dev = dev;
#elif defined(CONFIG_BME680)
  dev =  DEVICE_DT_GET_ONE(bosch_bme680);
  temp_dev = dev;
  humidity_dev = dev;
  pressure_dev = dev;
  
#else
  #error "no sensors specified "
#endif
  
  if (dev == NULL) {
    /* No such node, or the node does not have status "okay". */
    LOG_ERR("Error: no weather sensor device found.");
    return -1;
  }
  
  if (!device_is_ready(dev)) {
    LOG_ERR("Error: Device \"%s\" is not ready; "
	    "check the driver initialization logs for errors.",
	    dev->name);
    LOG_ERR("init %d init_res %d",dev->state->initialized, dev->state->init_res);
    return -1;
  }

  LOG_INF("Found device \"%s\"", dev->name);
  return 0;
}

static double meteorology_get_sensor(const struct device * dev, enum sensor_channel chan)
{
  double rc = -1.0;
  struct sensor_value value;
  int err;
  do {
    if(NULL == dev) {
      LOG_ERR("No sensor device");
      break;
    }
    err = sensor_sample_fetch(dev);
    if(0 > err) {
      LOG_ERR("%s sample fetch failed %d",dev->name, err);
      break;
    }
    err = sensor_channel_get(dev, chan, &value);
    if(0 > err) {
      LOG_ERR("%s channel %d get failed %d", dev->name, chan, err);
      break;
    }
    rc = sensor_value_to_double(&value);
  } while(0);
  return rc;
}
    
double meteorology_get_temperature()
{
  double rc = -1.0;
  if(NULL != temp_dev) {
    rc = meteorology_get_sensor(temp_dev,  SENSOR_CHAN_AMBIENT_TEMP);
  }
  return rc;
}

double  meteorology_get_humidity()
{
  double rc = -1.0;
  if(NULL != humidity_dev) {
    rc = meteorology_get_sensor(humidity_dev, SENSOR_CHAN_HUMIDITY);
  }
  return rc;
}

double meteorology_get_pressure()
{
  double rc = -1.0;
  if(NULL != pressure_dev) {
    rc = meteorology_get_sensor(pressure_dev, SENSOR_CHAN_PRESS);
  }
  return rc;
}

#endif // CONFIG_USER_METEOROLOGY_SENSOR
