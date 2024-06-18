# CH32 Ethernet

Arduino Library for the Ethernet peripheral of the WCH CH32V208.

It's still under _heavy_ development, so expect things to not work.

It needs a custom version of `arduino_core_ch32`, so be sure to add the following line to your `platformio.ini`:  
```
platform_packages = framework-arduino-openwch-ch32@https://github.com/LeoDJ/arduino_core_ch32
```

## Attribution
Based upon the works of:
- https://github.com/Xerbo/ch32-lwip
- https://github.com/stm32duino/STM32Ethernet