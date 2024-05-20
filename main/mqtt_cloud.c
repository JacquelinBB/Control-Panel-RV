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
#include "mqtt_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "mqtt_cloud.h"
#include "water_pump.h"

esp_mqtt_client_handle_t client;

int get_time = 0, get_led = 0, get_agua = 0, get_fluxo_time, get_fluxo, get_acesso_time, get_porta;
float get_distance = 0.0, get_cisterna = 0.0, get_caixa = 0.0;
char get_name[30], get_food1[5], get_food2[5], get_food3[5], get_drink[5];
bool topico_water, topico_fluxo, topico_door, topico_open_door, topico_food, topico_drink;

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
        xSemaphoreGive(mqtt_on_semaphore);
        msg_id = esp_mqtt_client_subscribe(client, "water/info", 0);
        msg_id = esp_mqtt_client_subscribe(client, "sala11/fluxo", 0);
        msg_id = esp_mqtt_client_subscribe(client, "esp32/door", 0);
        msg_id = esp_mqtt_client_subscribe(client, "esp32/open_door", 0);
        msg_id = esp_mqtt_client_subscribe(client, "set_food", 0);
        msg_id = esp_mqtt_client_subscribe(client, "set_drink", 0);
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
        if (strncmp(event->topic, "water/info", event->topic_len) == 0)
        {
            cJSON *root = cJSON_Parse(event->data);
            if (root != NULL)
            {
                cJSON *time_json = cJSON_GetObjectItem(root, "time");
                if (cJSON_IsNumber(time_json))
                {
                    get_time = time_json->valueint;
                }
                cJSON *led_json = cJSON_GetObjectItem(root, "led");
                if (cJSON_IsNumber(led_json))
                {
                    get_led = led_json->valueint;
                }
                cJSON *distancia_json = cJSON_GetObjectItem(root, "distancia");
                if (cJSON_IsNumber(distancia_json))
                {
                    get_distance = distancia_json->valuedouble;
                }
                cJSON *agua_json = cJSON_GetObjectItem(root, "agua");
                if (cJSON_IsNumber(agua_json))
                {
                    get_agua = agua_json->valueint;
                }
                cJSON *cisterna_json = cJSON_GetObjectItem(root, "cisterna");
                if (cJSON_IsNumber(cisterna_json))
                {
                    get_cisterna = cisterna_json->valuedouble;
                }
                cJSON *caixa_json = cJSON_GetObjectItem(root, "caixa");
                if (cJSON_IsNumber(caixa_json))
                {
                    get_caixa = caixa_json->valuedouble;
                }
                cJSON_Delete(root);
            }
            topico_water = true;
        }
        else if (strncmp(event->topic, "sala11/fluxo", event->topic_len) == 0)
        {
            cJSON *root = cJSON_Parse(event->data);
            if (root != NULL)
            {
                cJSON *time_json = cJSON_GetObjectItem(root, "time");
                if (cJSON_IsNumber(time_json))
                {
                    get_fluxo_time = time_json->valueint;
                }
                cJSON *fluxo_json = cJSON_GetObjectItem(root, "fluxo");
                if (cJSON_IsNumber(fluxo_json))
                {
                    get_fluxo = fluxo_json->valueint;
                }
                cJSON_Delete(root);
            }
            topico_fluxo = true;
        }
        else if (strncmp(event->topic, "esp32/door", event->topic_len) == 0)
        {
            cJSON *root = cJSON_Parse(event->data);
            if (root != NULL)
            {
                cJSON *time_json = cJSON_GetObjectItem(root, "time");
                if (cJSON_IsNumber(time_json))
                {
                    get_acesso_time = time_json->valueint;
                }
                cJSON *porta_json = cJSON_GetObjectItem(root, "sensor_porta");
                if (cJSON_IsNumber(porta_json))
                {
                    get_porta = porta_json->valueint;
                }
                cJSON_Delete(root);
            }
            topico_door = true;
        }
        else if (strncmp(event->topic, "esp32/open_door", event->topic_len) == 0)
        {
            cJSON *root = cJSON_Parse(event->data);
            if (root != NULL)
            {
                cJSON *name_json = cJSON_GetObjectItem(root, "name");
                if (cJSON_IsString(name_json)  && (name_json->valuestring != NULL))
                {
                    strncpy(get_name, name_json->valuestring, sizeof(get_name) - 1);
                    get_name[sizeof(get_name) - 1] = '\0'; 
                }
                cJSON_Delete(root);
            }
            topico_open_door = true;
        }
        else if (strncmp(event->topic, "set_food", event->topic_len) == 0)
        {
            cJSON *root = cJSON_Parse(event->data);
            if (root != NULL)
            {
                cJSON *food1_json = cJSON_GetObjectItem(root, "X");
                if (cJSON_IsString(food1_json)  && (food1_json->valuestring != NULL))
                {
                    strncpy(get_food1, food1_json->valuestring, sizeof(get_food1) - 1);
                    get_food1[sizeof(get_food1) - 1] = '\0';
                }
                cJSON *food2_json = cJSON_GetObjectItem(root, "Y");
                if (cJSON_IsString(food2_json)  && (food2_json->valuestring != NULL))
                {
                    strncpy(get_food2, food2_json->valuestring, sizeof(get_food2) - 1);
                    get_food2[sizeof(get_food2) - 1] = '\0';
                }
                cJSON *food3_json = cJSON_GetObjectItem(root, "Z");
                if (cJSON_IsString(food3_json)  && (food3_json->valuestring != NULL))
                {
                    strncpy(get_food3, food3_json->valuestring, sizeof(get_food3) - 1);
                    get_food3[sizeof(get_food3) - 1] = '\0';
                }
                cJSON_Delete(root);
            }
            topico_food = true;
        }
        else if (strncmp(event->topic, "set_drink", event->topic_len) == 0)
        {
            cJSON *root = cJSON_Parse(event->data);
            if (root != NULL)
            {
                cJSON *drink_json = cJSON_GetObjectItem(root, "drink");
                if (cJSON_IsString(drink_json)  && (drink_json->valuestring != NULL))
                {
                    strncpy(get_drink, drink_json->valuestring, sizeof(get_drink) - 1);
                    get_drink[sizeof(get_drink) - 1] = '\0';
                }
                cJSON_Delete(root);
            }
            topico_drink = true;
        }
        else if (strncmp(event->topic, "rv/pump", event->topic_len) == 0)
        {
            char *debug_data = malloc(event->data_len + 1);

            if (debug_data != NULL)
            {
                strncpy(debug_data, event->data, event->data_len);
                debug_data[event->data_len] = '\0';

                if (strcmp(debug_data, "pump") == 0)
                {
                    is_button_press = true;
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

void mqtt_publish_message(char *topic, char *message)
{
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(TAG_M, "Mensagem enviada, ID: %d", message_id);
}