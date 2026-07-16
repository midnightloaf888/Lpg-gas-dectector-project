#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_4

void app_main(void)
{
    gpio_reset_pin(LED_PIN);

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(LED_PIN, 1);
        printf("LED ON\n");
        sleep(1);
        
        gpio_set_level(LED_PIN, 0);
        printf("LED OFF\n");
        sleep(1);
    }
}