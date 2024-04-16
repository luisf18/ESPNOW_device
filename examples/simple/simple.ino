#include "ESPNOW_device.h"

// comment to upload to client
#define SERVER

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
  ESPNOW_device.update();
  if( !digitalRead(0) ){
    delay(250);
    ESPNOW_device.deinit();
  }
}
