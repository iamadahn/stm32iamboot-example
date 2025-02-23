#include "bsp.h"
#include "dwt.h"
#include "drivers/gpio/gpio.h"
#include "drivers/led/led.h"
#include "drivers/usart/usart.h"

#include <stm32f1xx_ll_utils.h>

#include <FreeRTOS.h>
#include <task.h>

#include <string.h>

static void led_heartbeat_task(void *pv_arg);
static void serial_task(void *pv_arg);

int main(void)
{
    /* Perform hardware initialisation */
    bsp_init();

    xTaskCreate(led_heartbeat_task, "led_heartbeat", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(serial_task, "serial", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    vTaskStartScheduler(); 

    return 0;
}

static void led_heartbeat_task(void *pv_arg)
{
    struct led led_heartbeat = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_13,
        .state = 0,
    };

    gpio_output_init(led_heartbeat.port, led_heartbeat.pin);

    while (1) {
        led_toggle(&led_heartbeat);
        vTaskDelay(500);
    }
}

static void serial_task(void *pv_arg)
{
    gpio_usart_tx_init(GPIOA, LL_GPIO_PIN_9);
    gpio_usart_rx_init(GPIOA, LL_GPIO_PIN_10);

    struct usart_config usart_dev = {
        .self = USART1,
        .apb_bus_freq = 72000000,
        .baudrate = 115200,
        .transfer_direction = LL_USART_DIRECTION_TX_RX,
        .data_width = LL_USART_DATAWIDTH_8B,
        .parity = LL_USART_PARITY_NONE,
        .stopbits = LL_USART_STOPBITS_1,
        .async_mode = 1,
        .irq_mode = 0,
    };
    usart_init(&usart_dev);

    while (1) {
        uint8_t msg[] = "Waiting for handshake from the host for 1 sec in blocking mode...\n\0";
        usart_block_transmit(&usart_dev, msg, strlen((char *)msg), 1000);
        vTaskDelay(1000);
    }
}
