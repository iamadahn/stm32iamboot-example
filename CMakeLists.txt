cmake_minimum_required(VERSION 3.17)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Add you project name
project(app C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_compile_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_compile_options(-lc -lm -lnosys -specs=nano.specs)
add_compile_options(-Wall)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-O0 -g)
endif ()

set(DEFINES
    -DSTM32F103xB
    -DUSE_FULL_LL_DRIVER
)

add_definitions(${DEFINES})

# Add Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bsp/cmsis/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bsp/ll/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sys
)

# Add you source file
file(GLOB SOURCES
    "*.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bsp/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bsp/cmsis/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bsp/ll/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sys/*.c"
)


# Add linker file
file(GLOB LINKER_SCRIPT "*.ld")
message(STATUS "Found linker script - ${LINKER_SCRIPT}")

# this options for C++
add_link_options(-specs=nano.specs -lc -lm)
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

# Drivers
add_subdirectory(${CMAKE_SOURCE_DIR}/src/drivers/gpio)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/drivers/led)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/drivers/usart)

target_link_libraries(${PROJECT_NAME}.elf
    gpio_driver
    led_driver
    usart_driver
)

# FreeRTOS library and config start
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
    INTERFACE
        ${CMAKE_SOURCE_DIR}/src/sys)
target_compile_definitions(freertos_config
    INTERFACE
        projCOVERAGE_TEST=0
)

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
# Select the cross-compile PORT
set(FREERTOS_PORT "GCC_ARM_CM3" CACHE STRING "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/freertos-kernel)

target_link_libraries(${PROJECT_NAME}.elf
    freertos_kernel
)

# Configure output artifacts
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")

# Custom command to build bootloader
# Bootloader requires 4 KiB of FLASH memory, which is 0x1000 in hex
# So the application can use all what's left
set(BOOTLOADER_CONFIG_DEFINES
    -DFLASH_BOOT_START_ADDR=0x08000000
    -DFLASH_APP_START_ADDR=0x080001000
    -DFLASH_PAGES=128           # The full amount of FLASH pages mcu has
    -DFLASH_PAGE_SIZE=1024
    -DUSE_USART1_PA9_PA10=1
    -DSTM32F103xB=1
)

add_custom_target(
    build_iamboot_bootloader
    COMMAND ${CMAKE_COMMAND} ${BOOTLOADER_CONFIG_DEFINES} -S ${CMAKE_SOURCE_DIR}/bootloader -B ${CMAKE_BINARY_DIR}/bootloader -GNinja 
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/bootloader
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/bootloader --target=build_iamboot_app
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building bootloader..."
)
