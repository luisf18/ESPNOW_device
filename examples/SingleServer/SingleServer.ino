#include "ESPNOW_device.h"

#define ESP32C3 // comente se não for esp32c3

#ifdef ESP32C3
#define LED 8
#define LED_ON LOW
#elif defined(ESP32)
#define LED 2
#define LED_ON HIGH
#elif defined(ESP8266)
#define LED 2
#define LED_ON LOW
#endif


void handle(espnow_device_event_t evt, int index ){

  //Serial.println( "\n\n" );

  if( evt == ESPNOW_EVT_SEND ){
    Serial.printf("[SEND][%d][%s]\n",index,ESPNOW_device.connection_index(index)->name);
  }else if( evt == ESPNOW_EVT_RECIVE ){
    Serial.printf("[RECIVE][%d][%s]\n",index,ESPNOW_device.connection_index(index)->name);
  }else if( evt == ESPNOW_EVT_NOTIFY ){
    Serial.printf("[NOTIFY][%d][%s]\n",index,ESPNOW_device.connection_index(index)->name);
  }else if( evt == ESPNOW_EVT_CONNECTED ){
    Serial.printf("[CONNECTED][%d][%s]\n",index,ESPNOW_device.connection_index(index)->name);
    digitalWrite(LED,LED_ON);
  }else if( evt == ESPNOW_EVT_DISCONNECTED ){
    Serial.printf("\n\n[DISCONNECTED][%d][%s]\n\n\n",index,ESPNOW_device.connection_index(index)->name);
    digitalWrite(LED,!LED_ON);
  }else if( evt == ESPNOW_EVT_SCAN_FOUND ){
    Serial.printf("\n\n[SCAN FOUND][%d][%s]\n\n\n",index,ESPNOW_device.connection_index(index)->name);
  }

  //Serial.printf( "[%s] last recive time: %d ms\n\n", ESPNOW_device.connection_index(index)->name, millis() - ESPNOW_device.connection_index(index)->last_time_recive );
}

void setup() {
  Serial.begin(115200);

  ESPNOW_device.set_handle_function(handle);

  // Server
  ESPNOW_device.begin_server( "Server", "1234", 1, 500, 1000 );

  // client
  //ESPNOW_device.begin_client( "Client", 1, 120, 3000 );
  //ESPNOW_device.connection_list_add( "Server", "1234" );
  //ESPNOW_device.enable_scan();

  pinMode(LED,OUTPUT);
  digitalWrite(LED,!LED_ON);
}

void loop() {
  ESPNOW_device.loop();
}
