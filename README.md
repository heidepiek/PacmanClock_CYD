

![PacmanClock_CYD](./images/cyd_pacmanclock.gif)



# PacmanClock_CYD  
Pacman Clock source code for CYD - ESP32 Cheap Yellow Display.

---

## NOTE:

Extensively modified by **Gerald Maurer** – Sept. 2024 — Version **3.1**  
Adapted to support **ESP32-2432S028** (CYD – Cheap Yellow Display)


---

## 🔧 Build Instructions

- **Compile using Arduino IDE** version **1.8.19**
- **ESP Board Manager**: Use **ESP32 version 2.0.17**
  - ⚠️ Newer versions (3.x.x or above) will not work properly
- **XT_DAC_Audio Library** version **4.2.1**
  - ✳️ Add this line to `XT_DAC_Audio.cpp` at line 27:
    ```cpp
    #include "soc/rtc_io_reg.h"
    ```
- **TFT_eSPI Configuration**:
  - Make sure the correct controller type is selected: `ILI9341_2_Driver`
  - Ensure the pin definitions match the CYD hardware (see example setup file `CYD.h`)

- **SPIFFS Initialization**:
  - Use the **"ESP32 Sketch Data Upload"** tool in Arduino IDE to initialize SPIFFS and upload files from the `data/` folder

---

## 🛠️ Modifications by Gerald Maurer

- Touch routine modified to use **XPT2046** library
- ESP32 pin definitions updated for CYD compatibility
- **DS3231 RTC** supported (optional)
  - Uses **GPIO 22 (SCL)** and **GPIO 27 (SDA)** — accessible via **CN1** connector
- Supports **display dimming** using onboard **LDR** (requires hardware change)

---

## 🔄 Personal Enhancements (Fork by [Your Name], Aug. 2025)

This fork adds several **visual** and **functional** upgrades, just for fun and customization:

- 📅 **Day and date** now displayed **above the clock**
- 💬 **Custom text line** displayed **below the time**
- 👾 Added **Pacman** and a **ghost icon** to the screen
- ⏱️ The **colon (:) blinks** to simulate ticking seconds (retro digital clock style)

> These additions aim to make the clock more playful and unique — feel free to expand further!

---

## 🚀 Quick Flash Option (No Arduino Setup Needed!)

To get a **working Pacman Clock** running immediately on your CYD board, use the included Windows batch file:

### `altered_ESP32_Pacman_Clock_CYD_V3_1-flash.bat`

What it does:
1. Prompts you to select the correct **COM port** for your CYD board
2. Automatically uploads:
   - The **precompiled .ino firmware**
   - The **SPIFFS data** (graphics, fonts, etc.)
3. No need to install libraries or open Arduino IDE — just double-click and go!

> ⚠️ Requires a Windows system and ESP32 USB drivers  
> Ideal for users who just want a **plug-and-play working clock**

---

## 🧪 Recommended Hardware Changes

To ensure full functionality of the dimming and audio features:

### 📉 LDR (Light Sensor)
- Replace **R15 (1.0MΩ)** with a **10KΩ** resistor  
  This enables proper operation of the onboard LDR for brightness control

### 🔊 Audio Output (Resistor Check)
Ensure the following resistor values are correct — replace if needed:
- `R4` → 4.7KΩ
- `R7` → 47KΩ
- `R8` → 22KΩ
- `R9` → 68KΩ

---
