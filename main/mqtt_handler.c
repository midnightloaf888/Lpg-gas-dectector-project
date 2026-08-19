#include <stdio.h>

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt_handler.h"
#include "lcd_driver.h"

// Office PC running Mosquitto
#define MQTT_BROKER   "mqtt://192.168.1.1:1883"

// Same topic used by the other intern
#define MQTT_TOPIC    "esp32/gas"

static const char *TAG = "MQTT_HANDLER";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(TAG, "MQTT connected successfully");
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGW(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_ERROR:
            mqtt_connected = false;
            ESP_LOGE(TAG, "MQTT connection error");
            break;

        default:
            break;
    }

    (void)handler_args;
    (void)base;
    (void)event_data;
}

void mqtt_start(void)
{
    if (mqtt_client != NULL) {
        return; // Already started
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to create MQTT client");
        return;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));

    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));

    ESP_LOGI(TAG, "MQTT client started");
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Mqtt Connected");
}

bool mqtt_is_connected(void)
{
    return mqtt_connected;
}

void mqtt_publish_value(int adc_raw)
{
    if (!mqtt_connected || mqtt_client == NULL) {
        printf("MQTT      : NOT CONNECTED\n");
        return;
    }

    char mqtt_message[16];
    snprintf(mqtt_message, sizeof(mqtt_message), "%d", adc_raw);

    int msg_id = esp_mqtt_client_publish(
        mqtt_client, MQTT_TOPIC, mqtt_message, 0, 1, 0);

    printf("MQTT      : SENT\n");
    printf("Topic     : %s\n", MQTT_TOPIC);
    printf("Message   : %s\n", mqtt_message);
    printf("Message ID: %d\n", msg_id);
}