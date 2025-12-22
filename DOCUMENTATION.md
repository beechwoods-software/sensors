# Sensors

This directory contains drivers, helpers and example code for sensors and
actuators used in Zephyr-based projects.

## Quick usage

Add the sensors library to your application's CMakeLists.txt:

```cmake
add_subdirectory(sensors)
target_link_libraries(app PRIVATE sensors)
```

Enable component configuration in your project's `prj.conf` (examples):

```
# Enable GPIO LED helpers
CONFIG_USE_GPIO_LED=y
# Enable motion sensor helper
CONFIG_USE_MOTION_SENSOR=y
```

## Patches

The `patches/` directory contains patch files required for specific boards
or Zephyr modules. Patch paths use the format `module/NN_description.patch`.
Apply with `patch -p1 < path/to/patch` from a Zephyr source tree when needed.

## Documentation

API and developer documentation is in the `doc/` directory and can be
generated with Doxygen (run `doxygen` from the `doc/` folder).

## Public headers and short notes

The `sensors/include` directory exposes small helper APIs used by apps:

- `gpio_led.h` — GPIO-driven LED helpers (`gpio_led_init()`,
  `gpio_led_set_led()`); enabled with `CONFIG_USE_GPIO_LED`.
- `ready_led.h` — higher-level ready/status LED helpers used on some
  boards.
- `button.h` — button state and callback registration (`button_init()`).
- `motion_sensor.h` — motion sensor init and callback (`motion_sensor_init()`);
  enabled with `CONFIG_USE_MOTION_SENSOR`.
- `stripled.h`, `pwmled.h`, `sevensegment.h` — LED strip, PWM LED and
  7-segment helpers.
- `rfid.h`, `pn532.h`, `mfrc522.h`, `st25dvxxk.h` — RFID / NFC related drivers.

## Examples and boards

Board overlays and configuration live under `boards/` and `zephyr/`.
See the board-specific `.overlay` files for GPIO assignments and device-tree
settings used by drivers in this directory.

## Next steps

- Add per-header API notes to the headers in `sensors/include` (short
  descriptions are present in header comments).
- If you want, I can run a repo-wide search to list all callers of the
  public APIs and update examples.
