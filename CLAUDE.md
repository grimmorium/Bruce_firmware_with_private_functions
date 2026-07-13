# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
- **Purpose**: ESP32-based multi-functional firmware designed for red team operations, supporting various wireless protocols (WiFi, BLE, RF, RFID, etc.).
- **Hardware Support**: Supports multiple platforms including M5Stack, Lilygo, and custom boards.
- **Framework**: Developed using the Arduino framework with PlatformIO as the build system.

## Development Architecture
- **Entry Point**: `src/main.cpp` is the main entry point for the application.
- **Core System (`src/core/`)**: 
    - Contains core logic, configuration management, and UI components (menus, themes).
    - Includes utilities for I2C discovery, SD card handling, and display rendering.
- **Modular Design (`src/modules/`)**: 
    - Each feature (WiFi, RF, BLE, etc.) is encapsulated within its own directory under `src/modules/`.
    - Modules are generally independent or loosely coupled through shared core components.
- **Configuration**: Hardware-specific configurations and constants are managed in `platformio.ini` using different environments to support multiple device types.

## Build & Development Commands
All commands assume the use of PlatformIO tools. Use `-e <env>` to specify the target hardware as defined in `platformio.in`.

| Action | Command | Description |
| :--- |15%| |
| **List Environments** | `pio list-envs` | See available device targets (e.g., `m5stack-cardputer`, `lilygo-t-embed`). |
| **Build** | `pio run -e <env>` | Compile the project for a specific board. |
| **Upload** | `pio run --target upload -e <env>` | Build and flash the binary to the target hardware. |
| **Monitor** | `pio device monitor` | Start the serial monitor (standard baud rate: 115200). |

## Key Files & Directories
- `platformio.ini`: Primary configuration for build environments, boards, and dependencies.
- `src/core/`: Central core logic including `menu_items`, `theme`, and hardware abstraction helpers.
- `src/modules/`: Feature modules (e.g., `rf/`, `wifi/`, `ble/`).
- `include/`: Header files shared across the project.
- `lib/`: Third-party libraries and local dependencies.
