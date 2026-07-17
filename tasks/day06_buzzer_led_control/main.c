#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_4
#define BUZZER_PIN GPIO_NUM_5

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_reset_pin(BUZZER_PIN);

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(LED_PIN, 1);
        gpio_set_level(BUZZER_PIN, 1);
        printf("LED ON | Buzzer ON\n");
        sleep(2);

        gpio_set_level(LED_PIN, 0);
        gpio_set_level(BUZZER_PIN, 0);
        printf("LED OFF | Buzzer OFF\n");
        sleep(2);
    }
}
