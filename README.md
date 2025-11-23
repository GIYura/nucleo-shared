### Brief

This repository contains a set of reusable software modules designed for different embedded platforms and applications. 
The modules are organized in a way that allows them to be easily adapted and used across multiple MCUs and project types.

### Structure

## 1. HAL (Hardware Abstraction Layer)

The `hal/include/` directory defines clean, MCU-agnostic APIs:

- `uart.h` — UART interface  
- `gpio.h` — GPIO interface
- `i2c.h` — I2C interface  
- `spi.h` — SPI interface  

## 2. Platforms (MCU-Specific Implementations)

Each MCU has its own folder under `platforms/`:

- `gpio.c`  
- `uart.c`  
- `spi.c`  
- `i2c.c`  

These files contain the actual register-level implementation of the HAL API.

## 3. Lib — Device Drivers and Services

These modules are fully portable and do not depend on MCU specifics.

### drivers/
Device drivers on top of HAL:

| Directory     | Description                    |
|---------------|--------------------------------|
| `adxl345/`    | Accelerometer driver (I2C/SPI) |
| `esp8266/`    | Wi-Fi module via AT commands   |
| `sx126x/`     | LoRa transceiver driver        |
| `radio/`      | Abstract radio interface       |
| `pwm/`        | PWM built on HAL timers        |


### services/

| Directory | Description        |
|-----------|--------------------|
| `log/`    | UART-based logger  |
| `cli/`    | Command line shell |


## 4. Core Components

System-level functionality:

- **freertos/** — FreeRTOS kernel sources  
- **mpu/** — memory protection utilities  
- **assert/** — project-wide assert system  

## 5. Utility Modules

Standalone helpers:

- **buffer/** — circular buffer  
- **delay/** — blocking delay functions  
- **event/** — event queue system  
- **utils/** — macros, helpers, small utilities  
