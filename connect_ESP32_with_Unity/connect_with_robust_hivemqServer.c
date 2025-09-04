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

#define WIFI_SSID "Mahmoud Kafafy"
#define WIFI_PASS "12345678"
#define MQTT_BROKER "mqtts://f446739580a645d9b26dc12166f78ed8.s1.eu.hivemq.cloud"
#define MQTT_USERNAME "Mahmoud"
#define MQTT_PASSWORD "123456789mM"

#define RELAY_GPIO GPIO_NUM_2

// HiveMQ Cloud CA certificate (replace with the actual certificate)
static const char *mqtt_ca_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";


static esp_mqtt_client_handle_t client = NULL;
static bool mqtt_connected = false; // Track MQTT connection status

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
            mqtt_connected = true; // Update connection status
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
            mqtt_connected = false; // Update connection status
            break;

        case MQTT_EVENT_ERROR:
            printf("MQTT error occurred.\n");
            break;

        default:
            break;
    }
}

// -------------------- Initialize Wi-Fi --------------------
void wifi_connect() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

// -------------------- Initialize MQTT --------------------
void mqtt_start() {
    esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {
        .address.uri = MQTT_BROKER,   // "mqtts://...:8883"
        .verification.certificate = mqtt_ca_cert,
    },
    .credentials = {
        .username = MQTT_USERNAME,
        .authentication.password = MQTT_PASSWORD,
    },
    .task.stack_size = 8192,
};

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        printf("ERROR: Failed to initialize MQTT client!\n");
        return;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_handler, NULL);
    esp_mqtt_client_start(client);
}

// -------------------- Heartbeat Task --------------------
void heartbeat_task(void *pvParameters) {
    while (1) {
        if (client != NULL && mqtt_connected) { // Check connection status
            esp_mqtt_client_publish(client, "unity/conveyor/control", "fwd", 0, 1, 0);
            printf("Sent 'fwd' to unity\n");
        } else {
            printf("MQTT client not connected, cannot send heartbeat\n");
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
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