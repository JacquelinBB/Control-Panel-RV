idf_component_register(SRCS "wifi.c" "main.c" "mqtt_cloud.c"
                    INCLUDE_DIRS ".")

target_add_binary_data(${COMPONENT_TARGET} "certs/aws_root_ca_pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate_pem_crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private_pem_key" TEXT)