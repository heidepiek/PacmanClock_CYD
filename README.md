# PacmanClock_CYD
Pacman Clock source code for CYD - ESP32 Cheap Yellow Display.
  
## Note the following:
  
Extensively modified by Gerald Maurer Sept. 2024  --  Version 3.1
   to support ESP32-2432S028 (CYD - Cheap Yellow Display)
   
## Build:
* Compile with Arduino Sketch IDE version 1.8.19.
* ESP Board Manager - select ESP32 version 2.0.17.
	* NOTE: Don't try to use newer versions (3.0.x or newer)
* Uses XT_DAC_Audio library Version 4.2.1.
	* NOTE: Add the following 'include' to XT_DAC_Audio.cpp at line 27
		* #include "soc/rtc_io_reg.h"
* Make sure the TFT_eSPI setup file selects the correct controller type (ILI9341_2_Driver)
		and the pin definitions are correct. See example User_Setup file CYD.h
## Modifications:
* Modified touch routine to use XPT2046 library.
* Modified ESP32 pin definitions as needed for CYD board.
* Supports DS3231 RTC if connected (optional), otherwise it uses NTP.
	* NOTE: DS3231 uses GPIO 22 (SCL) & 27 (SDA). CYD connector CN1. 
* Supports display dimming using the on-board LDR (see hardware changes).

## Hardware changes (recommended):
* LDR - Replace R15 1.0M resistor with a 10K resistor. Otherwise LDR will not work.
* Audio - Verify the following resistors for the audio are correct, if not, replace.
	* R4 - 4.7K
	* R7 - 47K
	* R8 - 22K
	* R9 - 68K
