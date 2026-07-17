#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_18

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    while (1)
    {
        int button_state = gpio_get_level(BUTTON_PIN);

        printf("Button Value = %d\n", button_state);

        sleep(2); 
    }
}