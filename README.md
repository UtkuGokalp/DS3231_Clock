# ⏰ STM32 RTC Clock System (DS3231 + HD44780 LCD)
This project is an embedded clock system built using an STM32 microcontroller communicating with a DS3231 Real-Time Clock (RTC) module over I2C.

The system displays time and date information on a 16x2 LCD (HD44780U controller) using a [custom-written LCD driver](https://github.com/UtkuGokalp/stm32-lcd-firmware). It also supports alarm configuration, time/date editing, and multiple display pages through a button-driven user interface.

The project was initially developed on an STM32F407 Discovery board and later ported to STM32F103C8T6 (Blue Pill) to create a more compact and portable implementation.

## 📦 Hardware
 - STM32F407 Discovery (development platform)
 - STM32F103C8T6 aka Blue Pill (final platform)
 - DS3231 RTC Module (I2C interface)
 - 16x2 LCD (HD44780U controller + extension chip)
 - Push Buttons for UI navigation and editing

**Note:** The included `.ioc` file targets STM32F103C8T6 (Blue Pill), not STM32F407 Discovery.

## ✨ Features
 - Real-time clock using DS3231 over I2C
 - Custom HD44780U LCD driver
 - Two LCD display pages
 - Alarm system
   - Enable/Disable alarm
   - Edit alarm time
 - Time and date editing via push buttons
 - 12h and 24h display format support (WIP)

## 🔧 System Architecture
The project was developed with modularity in mind and has multiple layers. This allows for easier portability and extendibility.
 - DS3231 and LCD low-level driver layers
 - Button debouncing layer
 - Application layer (UI, editing and alarm management logics)

## 🧠 Technical Highlights
 - Implemented RTC register parsing with BCD-to-decimal conversions
 - Designed a UI for user interaction and editing modes
 - Implemented software button debouncing
 - Ported firmware from STM32F4 to STM32F1 (handled peripheral differences)
 - Structured code to maintain separation between drivers and application logic

## 🛠️ Development Environment
 - STM32CubeIDE
 - STM32 HAL
 - st-flash CLI utility (for flashing Blue Pill)

## 🚀 How to Build
  1. Clone repository
  2. Open project in STM32CubeIDE
  3. Build the project
  4. Flash:
     - Directly from STM32CubeIDE for STM32F407-Discovery
     - Using `st-flash` for STM32F103 (Blue Pill)

## 📸 Demo
Photos and demonstration video will be added soon.

## 🔮 Future Improvements
 - Improve LCD firmware to ensure full compliance with HD44780U timing requirements
 - Complete and validate 12h/24h format handling
 - Design a custom PCB to integrate all the hardware
 - Develop a 3D printed enclosure
