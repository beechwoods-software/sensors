
cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_VERBOSE_MAKEFILE 1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

zephyr_library_named(sensors)
zephyr_include_directories(include)
zephyr_library_add_dependencies(zephyr_generated_headers)

zephyr_library_sources(src/sensor_log.c)
zephyr_library_sources_ifdef(CONFIG_USE_ADC src/adc.c)
zephyr_library_sources_ifdef(CONFIG_USE_BUTTON src/button.c)
zephyr_library_sources_ifdef(CONFIG_USE_GPIO_LED src/gpio_led.c)
zephyr_library_sources_ifdef(CONFIG_USE_KY40 src/ky40.c)
zephyr_library_sources_ifdef(CONFIG_USE_PWM_LED src/pwmled.c)
zephyr_library_sources_ifdef(CONFIG_USE_PWM_LED src/pwmled.c)
zephyr_library_sources_ifdef(CONFIG_USE_STRIP_LED src/stripled.c)
zephyr_library_sources_ifdef(CONFIG_USE_MOTION_SENSOR src/motion_sensor.c)
zephyr_library_sources_ifdef(CONFIG_USE_SEVEN_SEGMENT_DISPLAY src/sevensegment.c)
zephyr_library_sources_ifdef(CONFIG_USE_READY_LED src/ready_led.c)
zephyr_library_sources_ifdef(CONFIG_USE_METEOROLOGY_SENSORS src/meteorology.c)
zephyr_library_sources_ifdef(CONFIG_USE_LOWPOWER src/low_power.c)
zephyr_library_sources_ifdef(CONFIG_MFRC522 src/mfrc522.c)
zephyr_library_sources_ifdef(CONFIG_ST25DVXXK src/st25dvxxk.c)
zephyr_library_sources_ifdef(CONFIG_PN532 src/pn532.c)
zephyr_library_sources_ifdef(CONFIG_USE_MFRC522 src/mfrc522.c)
zephyr_library_sources_ifdef(CONFIG_ST25DVXXK src/st25dvxxk.c)

