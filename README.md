# PedalManager

![Compile Arduino files](https://github.com/MorGuux/PedalManager/workflows/Compile%20Arduino%20files/badge.svg)

## Arduino-powered Sim racing pedals manager.

### Supported hardware
 * analogRead (0-1023 10-bit)
 * ADS1115 (0-32767 15-bit)
 * HX711 load cell (0-131070 ~17-bit noise-free) (downscaled to 15-bit for HID)

### Features
* EEPROM-saved calibration values.
* Axis mapping (saturation, deadzone, 6 point adjustment)
* Linear recursive exponential smoothing filter (adjustable filter level)

### Dependancies
- Smoothed (https://github.com/MattFryer/Smoothed)
- Joystick (https://github.com/MHeironimus/ArduinoJoystickLibrary)
- ADSX115 fork (https://github.com/soligen2010/Adafruit_ADS1X15)
- HX711 (https://github.com/bogde/HX711)
- MultiMap (https://github.com/RobTillaart/MultiMap)

### How to use

Pedal.h class handles smoothing, mapping, calibration etc, but does not implement HID. This means you can use the class to direct the data elsewhere (maybe wireless, bluetooth etc.)

Each Pedal is initialised with a supported hardware type (ADS, analogRead or HX711), with a reference to each type being used when reading the pedal value. This may change in the future to remove hardware implementation, since there are many other ways of handling pedal data (hall effect sensors, potentiometers, other ADC modules etc) that may use different range, scalings, conversions. All raw pedal data should be provided as a 16-bit number (0-65535) for standardisation with HID axis'.

Methods are available to set calibration data such as deadzone, range, filter, mapping values etc. You may wish to implement a command based serial protocol to handle setup from software (see Simforge_Engineering_Pedals.ino). You can also implement auto-ranging calibration, percentage based deadzones and other features in your implementation.

### Command Table

| PC | Arduino reply | Description |
| --- | --- | --- |
| "i;v\n" | "i;v;1.0.0\r\n" | request firmware version |
|"i;c\n" | "i;c;Simforge Engineering\r\n" | request company name |
|"i;o;1\n" | "l;1234;4567;7890\r\n" | toggle live data (1 = on, 0 = off)|
|"c;0;a;00000\n" | "c;0;a;saved\r\n" | set upper deadzone for pedal 0 to 00000 |
|"c;1;b;01234\n" | "c;0;a;saved\r\n" | set lower deadzone for pedal 1 to 1234 |
|"c;1;c;32767\n" | "c;0;a;saved\r\n" | set upper range for pedal 1 to 32767 |
|"c;2;d;00000\n" | "c;0;a;saved\r\n" | set lower range for pedal 2 to 00000 |
|"c;2;f;00030\n" | "c;0;f;saved\r\n" | set filter value for pedal 2 to 30 |
|"c;0;z\n" | "c;0;z;reset\r\n" | factory reset calibration values |
