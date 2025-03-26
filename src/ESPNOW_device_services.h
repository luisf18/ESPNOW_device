#ifndef ESPNOW_DEVICE_SERVICES_H
#define ESPNOW_DEVICE_SERVICES_H

// os serviços padrões que podem ser transmitidos por espnow device

enum ESPNOW_device_service_t{
   ESPNOW_DEVICE_SERVICE_CHANNEL = 18,
   ESPNOW_DEVICE_SERVICE_TELEMETRY, //ESPNOW_DEVICE_SERVICE_NOTIFY,
   ESPNOW_DEVICE_SERVICE_SHELL
};

typedef struct{
  uint8_t cells   = 2;
  float   voltage = 0;
  uint8_t level   = 0;
  uint8_t channels_count = 0;
  int16_t channels[20];
} espnow_device_telemetry_t;

#endif