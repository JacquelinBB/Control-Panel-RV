idf_component_register(SRCS "ble.c" "screen.c" "sensor_mq2.c" "sensor_bme280.c" "water_pump.c" "rgb_led.c" "sensor_hcsr04.c" "wifi.c" "main.c" "mqtt_cloud.c" "water_pump.c"
                    INCLUDE_DIRS ".")

target_add_binary_data(${COMPONENT_TARGET} "certs/aws_root_ca_pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate_pem_crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private_pem_key" TEXT)