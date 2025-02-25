# stm32iamboot-example
Exmaple project with iamboot bootloader.

Used MCU is STM32F103CBT6, 128KB of ROM, 20KB of RAM.
The only needed configuration is located at bottom of CMakeLists.txt in the root folder of the project, no need to change anything in submodules.
```
set(BOOTLOADER_CONFIG_DEFINES
    -DFLASH_BOOT_START_ADDR=0x08000000
    -DFLASH_APP_START_ADDR=0x080001000
    -DFLASH_PAGES=128           # The full amount of FLASH pages mcu has
    -DFLASH_PAGE_SIZE=1024
    -DUSE_USART1_PA9_PA10=1
    -DSTM32F103xB=1
)
```

Run `build.sh` to build the project, it will produce artifacts for app, bootloader and host application in the build folder.
```
build/app.elf .hex .bin
build/bootloader/bootloader.elf .hex .bin
```
`build/bootloader/iamboot_app/iamboot_app` - is the host application to interact with bootloader.

Example usage:
`./build/bootloader/iamboot_app/iamboot_app /dev/ttyUSB0 build/app.bin`

PRs are welcomed since there are a lot of possible quality of life changes and not only them.

