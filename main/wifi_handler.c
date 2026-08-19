#include <string.h>
#include <stdio.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_log.h"

#include "wifi_handler.h"
#include "wifi_credentials.h"
#include "mqtt_handler.h"
#include "lcd_driver.h"

static const char *TAG = "WIFI_HANDLER";

// =====================================================
// WIFI SCAN
// =====================================================

static void scan_wifi_networks(void)
{
    printf("\n=====================================\n");
    printf("SCANNING NEARBY WIFI NETWORKS\n");
    printf("=====================================\n");

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
    };

    esp_err_t scan_result = esp_wifi_scan_start(&scan_config, true);

    if (scan_result != ESP_OK) {
        printf("Wi-Fi scan failed: %s\n", esp_err_to_name(scan_result));
        return;
    }

    uint16_t total_ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&total_ap_count));

    printf("Total networks found: %u\n\n", total_ap_count);

    uint16_t number = 20;
    wifi_ap_record_t ap_records[20];
    memset(ap_records, 0, sizeof(ap_records));

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_records));

    for (int i = 0; i < number; i++) {
        printf("%d. SSID: %s\n", i + 1, (char *)ap_records[i].ssid);
        printf("   Signal: %d dBm\n", ap_records[i].rssi);
        printf("   Channel: %d\n", ap_records[i].primary);
        printf("-------------------------------------\n");
    }

    printf("\nConfigured SSID: %s\n", WIFI_SSID);
    printf("=====================================\n\n");
}

// =====================================================
// WIFI EVENT HANDLER
// =====================================================

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi started");
        // Connection is triggered manually in wifi_init(), after the scan.
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disconnected =
            (wifi_event_sta_disconnected_t *)event_data;

        ESP_LOGW(TAG, "Wi-Fi disconnected. Reason code: %d", disconnected->reason);
        ESP_LOGI(TAG, "Retrying Wi-Fi connection...");

        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "Wi-Fi connected successfully");
        ESP_LOGI(TAG, "ESP32 IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Connecting to MQTT broker...");

        lcd_set_cursor(0, 0);
        lcd_print("Wifi Connected");

        vTaskDelay(pdMS_TO_TICKS(3000)); // Wait for 3 seconds before starting MQTT
        mqtt_start();
    }

    (void)arg;
}

// =====================================================
// WIFI INITIALIZATION
// =====================================================

void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized");

    // Scan first (useful for debugging), then connect.
    scan_wifi_networks();

    ESP_LOGI(TAG, "Trying to connect to: %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());
}