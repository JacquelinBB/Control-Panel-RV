#include <stdio.h>
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble.h"
#include "sensor_hcsr04.h"
#include "sensor_bme280.h"
#include "sensor_mq2.h"
#include "water_pump.h"
#include "screen.h"
#include "driver/gpio.h"
#include <stdlib.h>
#include <time.h>
#include "wifi.h"
#include "mqtt_cloud.h"

#include "esp_nimble_hci.h"

uint8_t ble_addr_type;
bool is_extern_activate = false;

static uint32_t generate_passkey(void) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    return 100000 + (rand() % 900000);
}

int water_level_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    ESP_LOGI(TAG_B, "Lendo nível de água: %.2f", water_level);
    os_mbuf_append(ctxt->om, &water_level, sizeof(water_level));
    return 0;
}

int environmental_sensor_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    char env_data[128];
    snprintf(env_data, sizeof(env_data), 
        "Temperatura: %.2f °C\n"
        "Umidade: %.2f %%\n"
        "Pressão Barométrica: %.2f hPa\n"
        "Presenca de Gas: %.1f ppm",
        temp, hum, press, mq2_value);
    ESP_LOGI(TAG_B, "%s", env_data);
    os_mbuf_append(ctxt->om, env_data, strlen(env_data));
    return 0;
}

int actuator_write_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t buf[1];
    os_mbuf_copydata(ctxt->om, 0, len, buf);
    
    uint8_t pump = buf[0];
    ESP_LOGI(TAG_B, "Valor recebido: %d", buf[0]);
    if (pump == 1 && !is_pump_on)
    {
        start_pump();
        is_extern_activate = true;
    }
    else if (pump == 0 && is_pump_on)
    {
        stop_pump();
        is_extern_activate = true;
    }
    return 0;
}

const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SENSOR_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { 
            {
                .uuid = BLE_UUID16_DECLARE(WATER_LEVEL_UUID),
                .access_cb = water_level_read_cb,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                .uuid = BLE_UUID16_DECLARE(ENVIRONMENTAL_UUID),
                .access_cb = environmental_sensor_read_cb,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                .uuid = BLE_UUID16_DECLARE(ACTUATOR_UUID),
                .access_cb = actuator_write_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            { 0 } // Fim das características
        },
    },
    { 0 } // Fim dos serviços
};

void ble_security_init(void) {
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_ONLY; // Passkey
    ble_hs_cfg.sm_bonding = 1; 
    ble_hs_cfg.sm_mitm = 1;       // Requer MitM (Man-in-the-Middle) protection
    ble_hs_cfg.sm_sc = 1;         // Ativar o Secure Connections (BLE 4.2+)
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;  // Distribui apenas a chave de criptografia
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC; // Recebe apenas a chave de criptografia
}

static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    int rc;
    struct ble_gap_adv_params adv_params;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            wifi_stop(); 
            mqtt_stop();
            ble_app_advertise();
            ESP_LOGI(TAG_B, "Conexão BLE estabelecida.");
        } else { // Se houve falha na conexão, reinicia a propaganda BLE
            ESP_LOGE(TAG_B, "Falha na conexão, tentando novamente...");
            ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
            ESP_LOGI(TAG_W, "ESP_WIFI_MODE_STA");
            wifi_init_sta();
            mqtt_start();
            info_mqtt();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG_B, "Conexão BLE perdida, iniciando novamente...");
        ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
        //ESP_LOGI(TAG_W, "ESP_WIFI_MODE_STA");
        //wifi_init_sta();
        //mqtt_start();
        //info_mqtt();
        //vTaskDelay(pdMS_TO_TICKS(100));
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_PASSKEY_ACTION:
        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            struct ble_sm_io pkey = {0};
            pkey.action = event->passkey.params.action;
            pkey.passkey = generate_passkey();
            ESP_LOGI(TAG_B, "Digite a Passkey: %06lu", (unsigned long)pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            if (rc != 0) {
                ESP_LOGE(TAG_B, "Falha ao injetar passkey, código de erro: %d", rc);
                return rc;
            }
        }
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        if (event->enc_change.status == 0) {
            ESP_LOGI(TAG_B, "Emparelhamento bem-sucedido");
        } else {
            ESP_LOGE(TAG_B, "Falha no emparelhamento, código de erro: %d", event->enc_change.status);
        }
        break;
    default:
        break;
    }
    return 0;
}

// Inicializa a propaganda BLE, para que o dispositivo fique visível e possa aceitar conexões
void ble_app_advertise(void) {
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_security_init();
    ble_app_advertise();
}

void ble_host_task(void *param) {
    nimble_port_run();
}

void info_mqtt(){
    char json_message[256];
    snprintf(json_message, sizeof(json_message), 
    "{\"temperature\": %.2f, \"humidade\": %.2f, \"pressure\": %.2f, \"gas\": %.1f, \"water_level\": %.2f, \"pump_status\": \"%s\"}", 
    temp, hum, press, mq2_value, water_level, is_pump_on ? "on" : "off");
    mqtt_publish_message("rv/info", json_message);
    vTaskDelay(pdMS_TO_TICKS(7000));
}