s# Sensors

A collection of files to use different kinds of sensors and actuators in zephyr


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
