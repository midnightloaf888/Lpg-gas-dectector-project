#pragma once

// Initializes Wi-Fi, scans nearby networks (for debugging), connects to
// the configured network, and starts MQTT once an IP is obtained.
void wifi_init(void);