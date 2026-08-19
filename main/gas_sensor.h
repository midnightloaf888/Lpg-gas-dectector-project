#pragma once

// Initializes ADC1 channel 6 (GPIO34) for the gas sensor.
void gas_sensor_init(void);

// Reads a single raw ADC value (0-4095).
int gas_sensor_read_raw(void);

// Converts a raw ADC value to a voltage (0-3.3V).
float gas_sensor_raw_to_voltage(int adc_raw);