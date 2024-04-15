#include "espnow_device_main.h"

//#define SERVER

void espnow_handle(espnow_device_event_t ev, int id){
  if( ev == ESPNOW_EVT_NOTIFY ){
    Serial.printf( "[ESPNOW DEVICE EVT NOTIFY %d]\n", id );
  }else if( ev == ESPNOW_EVT_PUBLIC_NOTIFY ){
    Serial.printf( "[ESPNOW DEVICE EVT PUBLIC NOTIFY %d]\n", id );
  }else if( ev == ESPNOW_EVT_DISCONNECTED ){
    //digitalWrite(2,LOW);
    Serial.printf( "[ESPNOW DEVICE EVT DISCONNECTED %d]\n", id );
  }else if( ev == ESPNOW_EVT_CONNECTED ){
    //digitalWrite(2,HIGH);
    Serial.printf( "[ESPNOW DEVICE EVT CONNECTED %d]\n", id );
  }else if( ev == ESPNOW_EVT_RECIVE ){
    Serial.printf( "[ESPNOW DEVICE EVT RECIVE %d]\n", id );
    //Serial.printf( " MAC: %s\n", mac2str( ESPNOW_device.connections[id].remote_mac ).c_str() );
    //Serial.printf( " Name: %s\n", ESPNOW_device.connections[id].frame_in.name );
    //Serial.printf( " Name RX: %s\n", ESPNOW_device.connections[id].frame_in.name_rx );
    //Serial.printf( " load.len: %d\n", ESPNOW_device.connections[id].frame_in.len );
    //Serial.printf( " load: %d\n", *( (int*) ESPNOW_device.connections[id].frame_in.body ) );
  }
}

void setup() {
  // inicia o LED
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);

  // Serial
  Serial.begin(115200);

  ESPNOW_device.set_led( 2 );
  ESPNOW_device.set_handle_function( espnow_handle );

  #ifdef SERVER
  Serial.println( "<< LOCAL SERVER >>" );
  ESPNOW_device.begin_server();
  #else
  Serial.println( "<< LOCAL CLIENT >>" );
  ESPNOW_device.begin_client();
  ESPNOW_device.connect( "SERVER", "1234" );
  #endif

  Serial.setTimeout(20);

}

void loop() {
  if( Serial.available() > 0 ){
    String cmd = Serial.readString();
    Serial.println(cmd);
    Serial.printf( "espnow_device_connection_counter %d", ESPNOW_device.connections_counter );
  }
  ESPNOW_device.update();
}
