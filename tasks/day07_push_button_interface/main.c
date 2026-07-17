#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define LED_PIN     GPIO_NUM_4
#define BUTTON_PIN  GPIO_NUM_18

void app_main(void)
{
    // Reset GPIOs
    gpio_reset_pin(LED_PIN);
    gpio_reset_pin(BUTTON_PIN);

    // Configure LED as output
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Configure Button as input
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    while (1)
    {
        int button_state = gpio_get_level(BUTTON_PIN);

        if (button_state == 0)
        {
            gpio_set_level(LED_PIN, 1);
            printf("Button Pressed - LED ON\n");
        }
        else
        {
            gpio_set_level(LED_PIN, 0);
            printf("Button Released - LED OFF\n");
        }

        usleep(100000);   // 100 ms delay
    }
}