#include "device_by_connection3.h"

//#define SERVER

void handle_espnow( espnow_device_event_t ev, int id ){
  if( ev == ESPNOW_EVT_NOTIFY ){
    Serial.printf( "[ESPNOW DEVICE] notify id: %d\n", id );
  }else if( ev == ESPNOW_EVT_PUBLIC_NOTIFY ){
    Serial.printf( "[ESPNOW DEVICE] Public notify id: %d\n", id );
  }else if( ev == ESPNOW_EVT_DISCONNECTED ){
    digitalWrite(2,LOW);
    Serial.printf( "[ESPNOW DEVICE] disconnect id: %d\n", id );
  }else if( ev == ESPNOW_EVT_CONNECTED ){
    digitalWrite(2,HIGH);
    Serial.printf( "[ESPNOW DEVICE] connect id: %d\n", id );
  }else if( ev == ESPNOW_EVT_RECIVE ){
    Serial.printf( "[ESPNOW DEVICE] recive id: %d\n", id );
    Serial.printf( " MAC: %s\n", mac2str( ESPNOW_device.connections[id].Remote.mac ).c_str() );
    Serial.printf( " Name: %s\n", ESPNOW_device.connections[id].frame_in.name );
    Serial.printf( " Name RX: %s\n", ESPNOW_device.connections[id].frame_in.name_rx );
    Serial.printf( " load.len: %s\n", ESPNOW_device.connections[id].frame_in.len );
    Serial.printf( " load: %d\n", *( (int*) ESPNOW_device.connections[id].frame_in.body ) );
  }
}

void setup() {
  
  // inicia o LED
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);

  // Serial
  Serial.begin(115200);

  // ESPNOW Device
  #ifdef SERVER
    ESPNOW_device.set_handle_function( handle_espnow );
    ESPNOW_device.beginServer("SERVER");
    ESPNOW_device.delay_ms_notify = 500;
  #else
    ESPNOW_device.set_handle_function( handle_espnow );
    ESPNOW_device.beginClient("CLIENT");
    ESPNOW_device.connect( "SERVER", "banana" );
  #endif

}

void loop() {
  ESPNOW_device.update();
}
