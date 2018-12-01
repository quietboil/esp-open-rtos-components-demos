# Working With FAT FS on SD Card

This demo shows how a program could use several of the [esp-open-rtos-components](https://github.com/quietboil/esp-open-rtos-components) to access the files stored on an SD card that is attached as an SPI slave device to the ESP8266.

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
* Card/Volume Comamnds:
  - `cm` - mounts the "volume".
  - `cs` - displays the size/capacity of the SD card.
* Directory Commands:
  - `dt` - prints directory tree that is rooted at the correct working directory. This command is limited to 16 levels of nested directories.
  - `dl` - lists the content of the current directory.
  - `dc newdir` - changes current working directory.
  - `dm dirname` - makes new directory in the current working directory.
  - `dr dirname` - removes sub-directory from the current directory.
* File Comamnds:
  - `fw filename` - writes text to a file. File will be created if it does not exist. After pressing <Enter> to execute the command start typing the text that will be saved into the file, or copy-and-paste some text into the serial console. When done terminate the input with <Ctrl-D>, which closes the file and terminates the command.
  - `fr filename` - reads file and dumps the text from it onto the serial console.
  - `fd filename` - deletes the file.

