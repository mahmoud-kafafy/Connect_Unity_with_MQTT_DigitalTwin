#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define WIFI_SSID "TERA"
#define WIFI_PASS "teratera1"
#define MQTT_BROKER "mqtt://broker.hivemq.com"

#define RELAY_GPIO GPIO_NUM_2

static esp_mqtt_client_handle_t client = NULL;

// -------------------- Wi-Fi Event Handler --------------------
static void wifi_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("Wi-Fi disconnected. Reconnecting...\n");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        printf("Wi-Fi connected and IP received.\n");
    }
}

// -------------------- MQTT Event Handler --------------------
static void mqtt_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
    esp_mqtt_event_handle_t event = data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("Connected to MQTT broker.\n");
            esp_mqtt_client_subscribe(client, "esp32/relay/control", 1);
            break;

        case MQTT_EVENT_DATA:
            printf("Message received: %.*s\n", event->data_len, event->data);

            if (strncmp(event->data, "on", event->data_len) == 0) {
                gpio_set_level(RELAY_GPIO, 0); // Turn relay ON
                printf("Relay turned ON.\n");
            } else if (strncmp(event->data, "off", event->data_len) == 0) {
                gpio_set_level(RELAY_GPIO, 1); // Turn relay OFF
                printf("Relay turned OFF.\n");
            }
            break;

        case MQTT_EVENT_DISCONNECTED:
            printf("Disconnected from MQTT broker.\n");
            break;

        default:
            break;
    }
}

// -------------------- Initialize Wi-Fi --------------------
void wifi_connect() {

    esp_netif_init(); //initializes the TCP/IP network interface layer

    esp_event_loop_create_default(); // Start the event loop from #include "esp_event.h"
                                    //It starts a FreeRTOS task to process these events in the background.


    esp_netif_create_default_wifi_sta(); /* Sets up the ip stack for Wi-Fi STA (Station) mode:
                                            DHCP client
                                            IP address handling
                                        */

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg); // It sets configuration fields for the Wi-Fi driver

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start(); // starts the Wi-Fi driver and enables Wi-Fi operations
}

// -------------------- Initialize MQTT --------------------
void mqtt_start() {
    esp_mqtt_client_config_t config = {
        .broker.address.uri = MQTT_BROKER
    };
    client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_handler, NULL);
    esp_mqtt_client_start(client);
}

// -------------------- Heartbeat Task --------------------
void heartbeat_task(void *pvParameters) {
    while (1) {
        esp_mqtt_client_publish(client, "esp32/heartbeat", "I'm alive!", 0, 1, 0);
        printf("Heartbeat sent.\n");
        vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds
    }
}

// -------------------- Main Application --------------------
void app_main() {
    nvs_flash_init();
    wifi_connect();

    // Configure relay pin as output
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << RELAY_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(RELAY_GPIO, 1); // Turn relay OFF by default

    vTaskDelay(pdMS_TO_TICKS(5000)); // Wait for Wi-Fi to connect

    mqtt_start();

    xTaskCreate(&heartbeat_task, "heartbeat", 4096, NULL, 5, NULL);
}
