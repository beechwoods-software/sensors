
# Sensors

A collection of files to use different kinds of sensors and actuators in zephyr
## Public API Summary

The documented public APIs (extracted from headers) are:

- `ready_led` (guarded by `CONFIG_USE_READY_LED`)
  - `int ready_led_init(void);` — initialize the ready LED, returns 0 on success.
  - `int ready_led_off(void);` — set LED off.
  - `int ready_led_on(void);` — set LED on.
  - `void ready_led_set(ready_led_speed_t speed);` — set blink speed.
  - `int ready_led_color(uint32_t red, uint32_t green, uint32_t blue);` — set color (0 / error codes per implementation).
  - `ready_led_speed_t ready_led_speed(void);` — get current speed.

- `button`
  - `int button_init(button_state_handler_t handler);` — initialize button with callback.
  - `void set_button_callback(button_state_handler_t cb);` — change callback.
  - Types: `button_state_t`, `button_callback_data_t`, `button_state_handler_t`.

- `sensor_log`
  - Module logger is registered as `SENSORS_LOG_MODULE_NAME` with Zephyr logging in `sensor_log.c`.

## Quick usage examples

- Initialize ready LED and set blink speed:

```c
if (ready_led_init() == 0) {
  ready_led_set(READY_LED_SHORT);
}
```


## Troubleshooting & TODOs

- Doxygen output is complete for `include/*.h` and `src/*.c` found by the `Doxyfile`. Open `sensors/doc/html/index.html` to inspect.
- TODO: consider adding explicit Doxygen comments for any undocumented functions in `src/` (Doxygen shows source-level code but richer @brief/@param/@return blocks improve the HTML pages).
- TODO: generate per-header Markdown docs under `sensors/docs/` if you want a repo-native README set (I can generate these from headers).



# Using
To use the sensor library add  
add_subdirectory(sensosr)  
and  
target_link_libraries(app PRIVATE sensors)  
to your applications CMakeLists.txt file

# Patches
The patches subdirectory contains patches needed. The format of the patches direcctory is  
module/patch where module is a directory whose name is the target directory for the patch. If the target is multiple directories below the root of the build  slashes ('/') are translated to underscores ('_') in the directory names  
As an example:  
      cd ~/zephyrprojects/zephyr  
      patch -p 1 < ../myapp/sensors/patches/zephyr/00_pico_w_ready_led.patch  
where myapp is the name of your application.  
The current patches are:  
zephyr/00_pico_w_ready_led.patch - This patch containes two functions that turn the onboard LED on and off.

# Documentation
The documentation can be generated from the doc directory.  
To create the documentation change directories to the doc directory and type doxygen. This will generate html and latex pages in the html and latex subdirectories.

# ready_led
The ready_led is an LED that is usually the onboard LED although other LEDs can be configured as the ready_led. It is used to signal status and events to the user.

# button
Buttons can be either accessible onboard buttons or external gpio buttons  
A callback is generated for each press or release of a button  
- Initialize button with callback:

```c
void my_button_cb(button_callback_data_t *bcd) {
  if (bcd->state == BUTTON_STATE_PRESSED) {
    // handle press
  }
}

button_init(my_button_cb);
```

# st25dv nfc driver
This driver implementation includes:
- Full support for I2C communication
- Interrupt handling with GPIO callback
- Fast Transfer Mode (FTM) support
- Energy Harvesting support
- Proper initialization and configuration
- Error handling and logging
- Device Tree support
Key features:
Interrupt handling through a work queue to avoid blocking in ISR context  
Support for both synchronous and asynchronous operations  
Configurable Fast Transfer Mode and Energy Harvesting  
Full register access for advanced configurations  
Support for reading and writing EEPROM memory  
Proper error handling and status reporting  
