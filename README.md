# SPI Master Communication Example

A comprehensive Zephyr RTOS-based SPI Master implementation that demonstrates efficient serial peripheral communication with support for multiple hardware platforms including Nordic nRF and STM32 microcontrollers.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Building the Project](#building-the-project)
- [Hardware Setup](#hardware-setup)
- [Configuration](#configuration)
- [Usage](#usage)
- [Code Structure](#code-structure)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Overview

This project demonstrates a robust SPI Master implementation using Zephyr RTOS. It sends periodic 32-byte messages to connected SPI slave devices, receives responses, and provides comprehensive debugging output including hex dumps of transmitted and received data.

The implementation showcases:
- Asynchronous SPI communication
- DMA-accelerated data transfers (STM32)
- Device Tree configuration for hardware abstraction
- Cross-platform compatibility (Nordic and STM32)
- Real-time debugging capabilities

## Features

- **Asynchronous SPI Communication**: Non-blocking SPI operations using Zephyr's async SPI API
- **Multi-Platform Support**: Compatible with Nordic nRF9151 and STM32 Nucleo-L4R5ZI boards
- **DMA Acceleration**: Efficient data transfer using Direct Memory Access (STM32 only)
- **Configurable Parameters**: Easy customization of SPI frequency, word size, and timing
- **Debug Output**:
  - Real-time message transmission logs
  - Hex dump of transmitted/received data
  - Timestamped operations
  - SPI configuration details at startup
- **Message Counter**: Incrementing counter to track successful transmissions
- **32-Byte Buffer**: Fixed-size buffer with automatic padding for consistent communication
- **GPIO-Controlled CS**: Manual chip select control for precise timing

## Hardware Requirements

### Supported Development Boards

| Board | Manufacturer | MCU | Status |
|-------|--------------|-----|--------|
| nRF9151 DK | Nordic Semiconductor | nRF9151 | Tested |
| Nucleo-L4R5ZI | STMicroelectronics | STM32L4R5ZI | Tested |

### External Hardware

- **SPI Slave Device**: Any SPI-compatible peripheral (sensor, memory, display, etc.)
- **Jumper Wires**: For connecting SPI signals between master and slave
- **Power Supply**: USB or external power for the development board

## Pin Configuration

### nRF9151 DK

| Signal | GPIO Pin | Arduino Pin |
|--------|----------|-------------|
| SCK    | P0.17    | -           |
| MOSI   | P0.18    | -           |
| MISO   | P0.19    | -           |
| CS     | P0.16    | -           |

### Nucleo-L4R5ZI

| Signal | GPIO Pin | Arduino Pin |
|--------|----------|-------------|
| SCK    | PA5      | D13         |
| MOSI   | PA7      | D11         |
| MISO   | PA6      | D12         |
| CS     | PA15     | -           |

**Note**: These pins must be connected to your SPI slave device. Ensure proper voltage level compatibility (3.3V logic).

## Software Requirements

### Required Software

- **nRF Connect SDK**: v2.7.2 or later (for Nordic boards)
- **Zephyr SDK**: v0.16.0 or later
- **CMake**: v3.13.1 or later
- **Python**: v3.8 or later (for build tools)
- **West Tool**: Zephyr's meta-tool

### Development Tools

- **VS Code** (Recommended) with nRF Connect extension
- **Segger J-Link** debugger support
- **Serial Terminal**: PuTTY, Tera Term, bs code integrated serial terminal ,or screen

## Installation

### 1. Install nRF Connect SDK

```bash
# Follow Nordic's official installation guide
# https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html
```

### 2. Clone the Repository

```bash
git clone <repository-url>
cd Spi-master
```

### 3. Initialize West Workspace (if not already done)

```bash
west init -l .
west update
```

## Building the Project

### Using Command Line

#### For nRF9151 DK:
```bash
west build -b nrf9151dk_nrf9151 -p
```

#### For Nucleo-L4R5ZI:
```bash
west build -b nucleo_l4r5zi -p
```

### Using VS Code (nRF Connect Extension)

1. Open Command Palette (Ctrl+Shift+P)
2. Select "nRF Connect: Create a new build configuration"
3. Choose your board
4. Click "Build Configuration"

### Build Options

Add custom configurations using menuconfig:
```bash
west build -t menuconfig
```

## Hardware Setup

### Wiring Diagram

Connect the SPI master (development board) to your SPI slave device:

```
Master (DK)          Slave Device
-----------          ------------
SCK       --------→  SCK
MOSI      --------→  MOSI (or SDI)
MISO      ←--------  MISO (or SDO)
CS        --------→  CS (or SS)
GND       --------   GND
```

### Important Notes

1. **Voltage Levels**: Ensure both devices operate at the same voltage (typically 3.3V)
2. **Pull-up/Pull-down**: Some SPI devices require pull-up resistors on CS or other lines
3. **Wire Length**: Keep wires as short as possible (< 15cm) for reliable high-speed communication
4. **Common Ground**: Always connect ground between master and slave

## Configuration

### SPI Parameters

Edit `src/main.c` to modify SPI configuration:

```c
static struct spi_config spi_cfg = {
    .frequency = 312500,    // 312.5 kHz (change as needed)
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER,
    .slave = 0,
    .cs = cs_ctrl,
};
```

### Supported Frequencies

- **nRF9151**: 125 kHz - 8 MHz
- **STM32L4R5**: 312.5 kHz - 40 MHz (with DMA)

### Project Configuration (prj.conf)

```conf
CONFIG_GPIO=y          # GPIO support for CS control
CONFIG_SPI=y           # SPI driver
CONFIG_SPI_ASYNC=y     # Asynchronous SPI API

# DMA Support (STM32 only)
CONFIG_DMA=y
CONFIG_DMA_STM32=y
CONFIG_SPI_STM32_DMA=y
```

### Device Tree Overlay

To add support for a new board, create `boards/<board_name>.overlay`:

```dts
my_spi_master:&spi1 {
    status = "okay";
    pinctrl-0 = <&spi1_default>;
    pinctrl-names = "default";
    cs-gpios = <&gpio0 16 GPIO_ACTIVE_LOW>;
};
```

## Usage

### 1. Flash the Firmware

```bash
west flash
```

### 2. Open Serial Monitor

```bash
# Linux/macOS
screen /dev/ttyACM0 115200

# Windows (adjust COM port)
putty -serial COM3 -serspeed 115200
```

### 3. Expected Output

```
SPI Master initialized.
=== SPI Configuration ===
Frequency: 312500 Hz
Operation: 0x00081004
Slave: 0
CS GPIO port: 0x20001000
CS GPIO pin: 16
CS GPIO flags: 0x0001
CS delay: 10
========================

Starting SPI loop...
TX HEX (32 bytes): 68 65 6C 6C 6F 20 66 72 6F 6D 20 6D 61 73 74 65 72 20 31 AA AA AA AA AA AA AA AA AA AA AA AA AA
[2000 ms] Sending [hello from master 1]...
TX HEX: 68 65 6C 6C 6F 20 66 72 6F 6D 20
OK, RX = [response from slave]

[3000 ms] Sending [hello from master 2]...
...
```

### Understanding the Output

- **TX HEX**: Hexadecimal dump of transmitted data
- **Timestamp**: System uptime in milliseconds
- **Message**: ASCII representation of sent data
- **RX**: Data received from slave device
- **0xAA padding**: Unused bytes are filled with 0xAA

## Code Structure

```
Spi-master/
├── src/
│   └── main.c              # Main application code
├── boards/
│   ├── nrf9151dk_nrf9151.overlay    # Nordic board config
│   └── nucleo_l4r5zi.overlay        # STM32 board config
├── CMakeLists.txt          # CMake build configuration
├── prj.conf                # Project Kconfig options
├── sample.yaml             # Test configuration
├── README.md               # This file
└── README.rst              # Original README

build/                      # Build output (generated)
```

### Key Files Explained

- **main.c**: Core SPI master implementation with message transmission loop
- **prj.conf**: Enables required Zephyr subsystems (GPIO, SPI, DMA)
- ***.overlay**: Device tree overlays for board-specific pin configurations
- **CMakeLists.txt**: Build system configuration

## Troubleshooting

### Problem: "SPI device not ready!"

**Solution**: Check that:
- Board overlay file exists for your board
- SPI peripheral is enabled in device tree
- Pin configuration is correct
- No hardware conflicts with other peripherals

### Problem: SPI ERROR -5 (EIO)

**Solution**:
- Verify wiring connections
- Check that slave device is powered
- Reduce SPI frequency
- Ensure CS pin is correctly configured

### Problem: No Response from Slave

**Solution**:
- Confirm slave device is in SPI mode
- Check SPI mode (CPOL/CPHA) compatibility
- Verify slave expects same word size (8-bit)
- Add delay after CS assertion (increase `cs_ctrl.delay`)

### Problem: Garbled Output

**Solution**:
- Check for proper ground connection
- Reduce wire length
- Add decoupling capacitors near SPI pins
- Lower SPI frequency

### Problem: Build Fails

**Solution**:
```bash
# Clean build
west build -t pristine

# Rebuild from scratch
west build -b <board> -p
```

## Advanced Topics

### Changing Message Rate

Modify the sleep duration in `main.c`:
```c
k_sleep(K_MSEC(1000));  // Change to K_MSEC(500) for 2 messages/sec
```

### Adjusting Buffer Size

Change `MSG_BUF_SIZE` at src/main.c:46:
```c
#define MSG_BUF_SIZE 64  // Increase from 32 to 64 bytes
```

### Adding Multiple Slave Devices

Create multiple CS pins in the overlay:
```dts
cs-gpios = <&gpio0 16 GPIO_ACTIVE_LOW>,
           <&gpio0 17 GPIO_ACTIVE_LOW>;
```

Then modify `spi_cfg.slave` in the code to select different devices.

## Performance Considerations

- **DMA**: Significantly reduces CPU load on STM32 (enabled by default)
- **Frequency**: Higher frequencies = faster transfers but less reliability
- **Buffer Size**: Larger buffers = more efficient DMA but higher memory usage
- **Async API**: Non-blocking operations allow concurrent tasks

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-board-support`)
3. Commit your changes (`git commit -am 'Add support for new board'`)
4. Push to the branch (`git push origin feature/new-board-support`)
5. Create a Pull Request

## License

This project is licensed under the Apache 2.0 License - see the LICENSE file for details.

## Resources

- [Zephyr Documentation](https://docs.zephyrproject.org/)
- [nRF Connect SDK Documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/)
- [SPI Bus Specification](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)
- [Device Tree Specification](https://devicetree.org/)

## Support

For issues, questions, or contributions, please open an issue on the GitHub repository.

---

**Author**: Waqar
**Last Updated**: January 2026
**Version**: 1.0.0
