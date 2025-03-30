#include "ESPNOW_device.h"

#define SERVER

void espnow_handler( espnow_device_event_t evt, ESPNOW_connection *cn ){
  switch( evt ){
    case ESPNOW_DEVICE__EVT_NULL:         Serial.println("[NULL]");         break;
    case ESPNOW_DEVICE__EVT_SEND:         Serial.println("[SEND]");         break;
    case ESPNOW_DEVICE__EVT_NOTIFY:       Serial.println("[NOTIFY]");       break;
    case ESPNOW_DEVICE__EVT_RECIVE:       Serial.printf("[RECIVE][ %s ]\n",cn->name);       break;
    case ESPNOW_DEVICE__EVT_CONNECTED:    Serial.printf("[CONNECTED][ %s ]\n",cn->name);    break;
    case ESPNOW_DEVICE__EVT_DISCONNECTED: Serial.printf("[DISCONNECTED][ %s ]\n",cn->name); break;
    case ESPNOW_DEVICE__EVT_SCAN_FOUND:
      Serial.printf("[Found][ %s - %s ]\n", cn->name, mac2str( cn->mac ).c_str() );
    break;
  }
}

void setup() {
  Serial.begin(115200);
  #ifdef SERVER
    ESPNOW_device.set_handler( espnow_handler );
    ESPNOW_device.begin_server("ROBOT","1234",1);
  #else
    ESPNOW_device.set_handler( espnow_handler );
    ESPNOW_device.begin_client("ESPNOW_client",1);
    ESPNOW_device.add_server( "ESPNOW_Server", "1234" );
    ESPNOW_device.add_server( "ESPNOW_Server2", "4321" );
  #endif
}

void loop() {
  #ifdef SERVER
  ESPNOW_device.loop();
  #endif
}
