#pragma once
#include <stdbool.h>

typedef struct {
    const char *status;   // "NORMAL", "WARNING", or "DANGER"
    bool led_on;
    bool buzzer_on;
} gas_state_t;

// Sets up the LED and buzzer GPIO pins.
void output_control_init(void);

// Compares adc_raw against the thresholds, drives the LED/buzzer
// pins accordingly, and returns the resulting state.
gas_state_t output_control_evaluate(int adc_raw);