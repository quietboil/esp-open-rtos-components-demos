# SPI Mode SD Card Demo

This application demos how a program would use [sdcard](https://github.com/quietboil/esp-open-rtos-components/tree/master/sdcard) API to access an SD Cards attached as an SPI slave to the ESP8266.

## Limitations

This demo iassumes that the SD card is the only slave on the SPI bus. While it uses [hspi](https://github.com/quietboil/esp-open-rtos-components/tree/master/hspi) component that was designed to allow multiple slaves attached to the master, this application left those capabilities for another demo.

## Installation

### Prerequisits

You need [esp-open-rtos-components](https://github.com/quietboil/esp-open-rtos-components) to build this demo. Download and extract downloaded ZIP somewhere, or clone the repo:
```sh
$ git clone https://github.com/quietboil/esp-open-rtos-components.git
```

Create the `local.mk` file that will be included by the `Makefile` and describe your build environment:
```makefile
# Directory where esp-open-rtos can be found on your system:
ESP_OPEN_RTOS_DIR = $(HOME)/esp-open-rtos
# Directory where esp-open-rtos-components can be found on your system:
COMPONENTS_DIR = $(HOME)/esp-open-rtos-components
# esptool parameters to flash the firmware:
FLASH_MODE = dio
FLASH_SPEED = 80
FLASH_SIZE = 32
ESPBAUD = 460800
```
> **Note** that the `esptool` configration above is what works for the NodeMCU board that I use for development. The board is equipped with the CH340 USB to serial chip, which has the maximum baud rate of 2000000 and yet the maximum flash speed that works with my board is 460800. YMMV.

Then just build and flash the firmware:
```sh
$ make
$ make flash
```

## Usage

Once the firmware has booted it would expect a command on the serial console. It initializes the serial port to 115200 baud, so for example to using `picocom` one might connect to it like this:
```sh
$ picocom -b 115200 --omap crcrlf /dev/ttyUSB0
```

Once connected, if you like, you can press <Enter> to force the "command line" prompt to appear as you might not see it if `picocom` was launched just a bit late to see the initial prompt.

The set of command it expects can be seen in the `main.c`. Here's a brief description of them:
- `i` - initializes the SD card. Try it without the card in the slot, to see the error code. You need to initialize the card before you could read/write/erase it.
- `c` - set "slow communication" flag on the card descriptor. When this flag is set HSPI clock will be set to 100kHz when it "talks" to the SD card. If you want to attach logic analyzer to the SPI bus that is not capable of sampling the default 20MHz SPI clock, this "slow" mode would help.
- `C` - remove "slow communication" flag from the card descriptor. Without this flag (default state) SPI clock runs at 20MHz.
- `a 1234` - sets "address" of the sector for the subsequent read/write/erase commands. Note that the [sdcard](https://github.com/quietboil/esp-open-rtos-components/tree/master/sdcard) component sets sector size to 512 bytes for non-SDHC card to provide a uniform access API.
- `r` - reads the content of the sector (512 bytes) which address was set before by the `a` command.
- `w` - write a generated patters of 512 bytes into the sector which address was set by the `a` command.
- `e` - erases the sector which address was set by the `a` command.
