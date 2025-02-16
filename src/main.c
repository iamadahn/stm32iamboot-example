#include "bsp.h"
#include "dwt.h"
#include "drivers/gpio/gpio.h"
#include "drivers/led/led.h"
#include "drivers/usart/usart.h"

#include <stm32f1xx_ll_utils.h>

int main(void)
{
    SCB->VTOR = 0x8001000;
    /* Perform hardware initialisation */
    bsp_init();  

    struct led led_heartbeat = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_13,
        .state = 0,
    };

    gpio_output_init(led_heartbeat.port, led_heartbeat.pin);

    struct usart_config usart_dev = {
        .self = USART1,
        .apb_bus_freq = 64000000,
        .baudrate = 115200,
        .transfer_direction = LL_USART_DIRECTION_TX_RX,
        .data_width = LL_USART_DATAWIDTH_8B,
        .parity = LL_USART_PARITY_NONE,
        .stopbits = LL_USART_STOPBITS_1,
        .async_mode = 0,
        .irq_mode = 0,
    };
    usart_init(&usart_dev);

    while (1) {
        led_toggle(&led_heartbeat);
        LL_mDelay(100);
    }

    return 0;
}
