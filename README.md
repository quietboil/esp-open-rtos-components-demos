# Modular Components for the esp-open-rtos

This is a collection of demo applications that show some aspects of [esp-open-rtos-components](https://github.com/quietboil/esp-open-rtos-components) usages. These applications started their life as test dummies and were used to ensure that the components do what I expected of them. After some minor tidying they are collected here to be used as examples and maybe starting points for the application that are relying on the [esp-open-rtos-components](https://github.com/quietboil/esp-open-rtos-components).

- [SPI Mode SD Card Demo](sdcard_demo) shows how to initialize an SD card that is attached to ESP8266 as an SPI slave, read, write and erase sectors.
- [FAT FS on SD Card Demo](fatfs_sdcard_demo) shows how to access (mount) an SD card with a FAT filesystem on it, list content of directories, create or remove directories, create/write and read files.
