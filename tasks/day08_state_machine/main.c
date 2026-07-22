#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_25

typedef enum
{
    STATE_NORMAL,
    STATE_WARNING,
    STATE_DANGER
} system_state_t;

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    system_state_t current_state = STATE_NORMAL;

    int previous_button_state = gpio_get_level(BUTTON_PIN);

    printf("Current State: NORMAL\n");

    while (1)
    {
        int current_button_state = gpio_get_level(BUTTON_PIN);

        if (previous_button_state == 1 &&
            current_button_state == 0)
        {
            if (current_state == STATE_NORMAL)
            {
                current_state = STATE_WARNING;
                printf("Current State: WARNING\n");
            }
            else if (current_state == STATE_WARNING)
            {
                current_state = STATE_DANGER;
                printf("Current State: DANGER\n");
            }
            else
            {
                current_state = STATE_NORMAL;
                printf("Current State: NORMAL\n");
            }

            usleep(200000);
        }

        previous_button_state = current_button_state;

        usleep(20000); 
    }
}