#pragma once
#include <stdbool.h>

// Starts the MQTT client and connects to the broker.
// Safe to call more than once - only creates the client the first time.
void mqtt_start(void);

// Returns true if MQTT is currently connected to the broker.
bool mqtt_is_connected(void);

// Publishes an integer ADC value to the configured topic.
// Prints "NOT CONNECTED" to serial and does nothing else if MQTT is down.
void mqtt_publish_value(int adc_raw);