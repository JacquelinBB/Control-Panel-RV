#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "mqtt_cloud.h"
#include "water_pump.h"
#include "screen.h"

esp_mqtt_client_handle_t client;

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG_M, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG_M, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_M, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "rv/pump", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_M, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_M, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_M, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_M, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_M, "MQTT_EVENT_DATA");
        if (strncmp(event->topic, "rv/pump", event->topic_len) == 0)
        {
            char *debug_data = malloc(event->data_len + 1);

            if (debug_data != NULL)
            {
                strncpy(debug_data, event->data, event->data_len);
                debug_data[event->data_len] = '\0';

                if (strcmp(debug_data, "1") == 0)
                {
                    start_pump();
                }
                else if (strcmp(debug_data, "0") == 0)
                {
                    stop_pump();
                }
                free(debug_data);
            }
            else
            {
                printf("Erro ao alocar memória.\n");
            }
        }
        else
        {
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_M, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG_M, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG_M, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_start()
{
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = BROKER_URL,
        .broker.verification.certificate = (const char *)aws_root_ca_pem_start,
        .credentials = {
            .authentication = {
                .certificate = (const char *)certificate_pem_crt_start,
                .key = (const char *)private_pem_key_start,
            },
        }};

    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_stop() {
    ESP_LOGI(TAG_M, "Parando o MQTT...");

    if (client) {
        esp_mqtt_client_stop(client); 
        esp_mqtt_client_destroy(client);
        client = NULL; 
    } else {
        ESP_LOGW(TAG_M, "Cliente MQTT já estava nulo. Nenhuma ação necessária.");
    }

    ESP_LOGI(TAG_M, "MQTT parado com sucesso.");
}

void mqtt_publish_message(char *topic, char *message)
{
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(TAG_M, "Mensagem enviada, ID: %d", message_id);
}