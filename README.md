# PedalManager

![Compile Arduino files](https://github.com/MorGuux/PedalManager/workflows/Compile%20Arduino%20files/badge.svg)

ADS1115-connected pedals, controlled with software. EEPROM-saved calibration values.

| PC | Arduino reply | Description |
| --- | --- | --- |
| "i;v\n" | "i;v;1.0.0\r\n" | request firmware version |
|"i;c\n" | "i;c;Simforge Engineering\r\n" | request company name |
|"i;o\n" | "l;1234;4567;7890\r\n" | toggle live data |
|"c;0;a;00000\n" | "c;0;a;saved\r\n" | set upper deadzone for pedal 0 to 00000 |
|"c;1;b;01234\n" | "c;0;a;saved\r\n" | set lower deadzone for pedal 1 to 1234 |
|"c;1;c;32767\n" | "c;0;a;saved\r\n" | set upper range for pedal 1 to 32767 |
|"c;2;d;00000\n" | "c;0;a;saved\r\n" | set lower range for pedal 2 to 00000 |
|"c;2;f;00030\n" | "c;0;f;saved\r\n" | set filter value for pedal 2 to 30 |
|"c;0;z\n" | "c;0;z;reset\r\n" | factory reset calibration values |
