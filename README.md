# _Control Panel_

Painel de controle para motorhome.

## Dependêncis
- Utilizar a extensão Espressif IDF
- Componentes: ESP32-WROOM-D, HC-SR04, BME280, MQ2, ILI9488 Relé, Protoboards, Bomba de água 5V e cabos.

## Conteúdo das pastas

Categorização

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   ├── ble.c
│   ├── ble.h
│   ├── mqtt_cloud.c
│   ├── mqtt_cloud.h
│   ├── wifi.c
│   ├── wifi.h
│   ├── screen.c
│   ├── screen.h
│   ├── water_pump.c
│   ├── water_pump.h
│   ├── sensor_mq2.c
│   ├── sensor_mq2.h
│   ├── sensor_bme280.c
│   ├── sensor_bme280.h
│   ├── sensor_hcsr04.c
│   ├── sensor_hcsr04.h
│   ├── Kconfig.projbuild
│   └── main.c
├── components
│   ├── bme280
│   ├── lvgl_esp32_drivers
│   └── lvgl
├── certs
│   ├── aws_root_ca_pem
│   ├── certificate_pem_crt
│   └── private_pem_key
├── fotos
│   ├── fluxograma
│   └── circuito
├── partitions.csv
├── .gitignore
├── LICENSE
└── README.md  This is the file you are currently reading
```
