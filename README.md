# TMC2209 Universal Driver

Low-level universal C driver for the **TMC2209 stepper motor driver**, designed for embedded systems and microcontrollers.

The driver provides UART communication with the TMC2209 and allows configuration of motor current, microstepping, chopper mode, shaft direction and other driver parameters.

The hardware-specific UART implementation is provided by the application through callback functions, making the driver independent of a particular MCU or HAL.

## Features

* UART communication with TMC2209
* TMC2209 UART packet generation
* CRC-8 calculation and verification
* Register read/write operations
* Default driver initialization
* RMS motor current configuration
* IHOLD / IRUN / IHOLDDELAY configuration
* Microstep configuration
* Interpolation configuration
* Shaft direction inversion
* StealthChop / SpreadCycle selection
* CHOPCONF `TOFF` control
* Driver enable / disable
* VACTUAL control
* Hardware-independent UART interface
* No dependency on STM32 HAL or other MCU-specific libraries

## Supported Registers

The driver currently uses the following TMC2209 registers:

| Register     | Address | Purpose                     |
| ------------ | ------: | --------------------------- |
| `GCONF`      |  `0x00` | Global configuration        |
| `IHOLD_IRUN` |  `0x10` | Motor current configuration |
| `VACTUAL`    |  `0x22` | Velocity configuration      |
| `CHOPCONF`   |  `0x6C` | Chopper configuration       |

## Architecture

The driver is divided into two logical layers.

### Transport layer

Responsible for communication with the TMC2209:

* UART packet construction
* CRC calculation
* Register read/write operations
* UART callbacks

### Driver layer

Responsible for higher-level TMC2209 configuration:

* Current control
* Microstepping
* Chopper configuration
* Shaft direction
* Driver enable/disable
* Initialization

The driver does not initialize the MCU UART peripheral itself.

Instead, the application provides UART functions through callbacks:

```c
typedef uint8_t (*tmc2209_uart_send)(uint8_t *data, uint8_t length);
typedef uint8_t (*tmc2209_uart_receive)(uint8_t *data, uint8_t length);
```

This allows the same driver to be used with different microcontrollers and UART implementations.

## Driver Handle

The driver uses a handle containing the UART address, communication callbacks and internal configuration state.

```c
tmc2209_t motorX = {
    .addr = UART_ADDRESS_0,
    .send = uart_send,
    .receive = uart_receive,
};
```

The application is responsible for providing the correct UART transport functions.

## Initialization

The driver provides a default initialization function:

```c
tmc2209_init_default(&motorX);
```

The default configuration initializes:

* `GCONF`
* `IHOLD_IRUN`
* `CHOPCONF`

The default current configuration is:

```text
IHOLD      = 550 mA
IRUN       = 1100 mA
IHOLDDELAY = 6
```

The exact motor current depends on the TMC2209 board configuration, sense resistor and voltage-sense configuration.

## Current Control

The driver provides a convenient RMS current interface instead of requiring the user to manually calculate the `CS` value.

### Set complete current configuration

```c
tmc2209_set_current(&motorX, 550, 1100, 6);
```

Parameters:

```text
IHOLD      = 550 mA
IRUN       = 1100 mA
IHOLDDELAY = 6
```

### Change IHOLD

```c
tmc2209_set_ihold(&motorX, 550);
```

### Change IRUN

```c
tmc2209_set_irun(&motorX, 1100);
```

### Change IHOLDDELAY

```c
tmc2209_set_ihold_delay(&motorX, 6);
```

The driver maintains a software shadow of the `IHOLD_IRUN` register so individual parameters can be changed without unnecessarily losing the other fields.

## Microstepping

Microstepping can be configured through:

```c
tmc2209_set_microsteps(&motorX,TMC2209_MICROSTEP_16,TMC2209_INTPOL_ENABLE);
```
Supported microstep values:

```text
TMC2209_MICROSTEP_256 
TMC2209_MICROSTEP_128
TMC2209_MICROSTEP_64 
TMC2209_MICROSTEP_32 
TMC2209_MICROSTEP_16
TMC2209_MICROSTEP_8
TMC2209_MICROSTEP_4
TMC2209_MICROSTEP_2
TMC2209_MICROSTEP_FULLSTEP
```

Interpolation can be enabled or disabled independently.

```c
TMC2209_INTPOL_ENABLE
TMC2209_INTPOL_DISABLE
```

## Shaft Direction

The shaft direction can be inverted through the `GCONF.SHAFT` bit:

```c
tmc2209_inverse_direction(&motorX,TMC2209_SHAFT_INVERTED);
```
Normal direction:

```c
TMC2209_SHAFT_NORMAL
```

Inverted direction:

```c
TMC2209_SHAFT_INVERTED
```

## Chopper Mode

The driver supports:

```c
TMC2209_MODE_STEALTHCHOP
TMC2209_MODE_SPREADCYCLE
```

Example:

```c
tmc2209_set_chopper_mode(&motorX,TMC2209_MODE_STEALTHCHOP);
```

## Driver Enable / Disable

The driver uses the `CHOPCONF.TOFF` field to enable or disable the power stage.

Enable:

```c
tmc2209_enable(&motorX);
```

Disable:

```c
tmc2209_disable(&motorX);
```

Internally:

```text
TMC2209_TOFF_DISABLE    
TMC2209_TOFF_ENABLE
```

## VACTUAL

The driver provides access to the TMC2209 `VACTUAL` register:

```c
tmc2209_set_vactual(&motorX, 10000);
```

`VACTUAL` can be used to command internal velocity generation without external STEP pulses.

This functionality is mainly useful for testing and specific operating modes. For normal stepper motor positioning and motion control, STEP/DIR should generally be used.

## UART Addressing

The TMC2209 supports up to four UART addresses:

```c
UART_ADDRESS_0
UART_ADDRESS_1
UART_ADDRESS_2
UART_ADDRESS_3
```

Example:

```c
motorX.addr = UART_ADDRESS_0;
```

This allows multiple TMC2209 devices to share the same UART bus when their addresses are configured appropriately.

## CRC

TMC2209 UART communication uses CRC-8.

The driver calculates CRC automatically for transmitted packets and verifies CRC for received packets.

Register reads return:

```c
typedef struct
{
    uint32_t value;
    uint8_t error;
} tmc2209_read_result_t;
```

Example:

```c
tmc2209_read_result_t result;

result = tmc2209_read_register(&motorX, GCONF);

if (!result.error)
{
    uint32_t gconf = result.value;
}
```

If the CRC verification fails:

```text
error = 1
```

If the received packet is valid:

```text
error = 0
```

## Example UART Integration

The driver does not depend on a particular UART peripheral.

For example, an MCU application can provide:

```c
uint8_t uart_send(uint8_t *data, uint8_t length)
{
    /* MCU-specific UART transmission */
}

uint8_t uart_receive(uint8_t *data, uint8_t length)
{
    /* MCU-specific UART reception */
}
```

Then initialize the driver:

```c
tmc2209_t motorX = {
    .addr = UART_ADDRESS_0,
    .send = uart_send,
    .receive = uart_receive,
};
```

After that:

```c
tmc2209_init_default(&motorX);
```

The TMC2209 driver itself remains independent of the MCU UART implementation.

## Important

The driver is intended to be a **low-level hardware-independent driver**.

It does not:

* initialize UART peripherals
* configure GPIO pins
* configure clocks
* generate STEP pulses
* configure timers
* depend on STM32 HAL
* depend on a specific microcontroller

These responsibilities belong to the application or hardware abstraction layer.

## Requirements

The driver requires only:

* C99-compatible compiler
* `<stdint.h>`

No external libraries are required.

## Project Structure

A typical project can be organized as:

```text
tmc2209/
├── Inc/
│   └── tmc2209.h
│
├── Src/
│   └── tmc2209.c
│
└── README.md
```

## Current API

### Initialization

```c
void tmc2209_init_default(tmc2209_t *handler);
```

### Current control

```c
void tmc2209_set_current(tmc2209_t *handler, uint16_t ihold, uint16_t irun, uint8_t ihold_delay);
void tmc2209_set_ihold(tmc2209_t *handler, uint16_t ihold);
void tmc2209_set_irun(tmc2209_t *handler, uint16_t irun);
void tmc2209_set_ihold_delay(tmc2209_t *handler, uint16_t ihold_delay);
```

### Motion control

```c
void tmc2209_inverse_direction(const tmc2209_t *handler, tmc2209_shaft_t direction);
void tmc2209_set_microsteps(const tmc2209_t *handler, tmc2209_microstep_t microstep, tmc2209_intpol_t intpol);
void tmc2209_set_vactual(const tmc2209_t *handler, int32_t value);
```

### Chopper / driver state

```c
void tmc2209_set_chopper_mode(const tmc2209_t *handler, tmc2209_chop_mode_t chop_mode);
void tmc2209_set_toff(const tmc2209_t *handler, tmc2209_toff_t toff_value);
void tmc2209_enable(const tmc2209_t *handler);
void tmc2209_disable(const tmc2209_t *handler);
```

## Status

The driver is currently under development.

The current implementation focuses on the core TMC2209 UART communication and configuration functionality.

Possible future additions include:

* `PWMCONF`
* `TCOOLTHRS`
* `TPWMTHRS`
* `SGTHRS`
* `DRV_STATUS`
* `IFCNT`
* `IOIN`
* automatic driver status/error handling
* additional diagnostics
* improved communication error reporting

## License

This project is provided for educational and embedded-development purposes.
