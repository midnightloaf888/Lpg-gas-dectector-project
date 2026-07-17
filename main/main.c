#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define GREEN_LED  GPIO_NUM_4
#define YELLOW_LED GPIO_NUM_5
#define RED_LED    GPIO_NUM_18

void app_main(void)
{
    gpio_reset_pin(GREEN_LED);
    gpio_reset_pin(YELLOW_LED);
    gpio_reset_pin(RED_LED);

    gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(YELLOW_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(GREEN_LED, 1);
        gpio_set_level(YELLOW_LED, 0);
        gpio_set_level(RED_LED, 0);
        printf("GREEN LED ON\n");
        sleep(2);

        gpio_set_level(GREEN_LED, 0);
        gpio_set_level(YELLOW_LED, 1);
        gpio_set_level(RED_LED, 0);
        printf("YELLOW LED ON\n");
        sleep(2);

        gpio_set_level(GREEN_LED, 0);
        gpio_set_level(YELLOW_LED, 0);
        gpio_set_level(RED_LED, 1);
        printf("RED LED ON\n");
        sleep(2);
    }
}