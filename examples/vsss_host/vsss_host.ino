#include "ESPNOW_device.h"

enum ESPNOW_device_service{
   ESPNOW_DEVICE_SERVICE_CHANNEL = 18,
   ESPNOW_DEVICE_SERVICE_NOTIFY,
   ESPNOW_DEVICE_SERVICE_SHELL,
   ESPNOW_DEVICE_SERVICE_NONE
};

String ESPNOW_device_cmd = "";

typedef struct{
  uint8_t cells   = 2;
  float   voltage = 0;
  uint8_t level   = 0;
} my_notify_t;


// ==============================================================================
// espnow_device_print
// ==============================================================================
void espnow_device_print( const char *input ){
  ESPNOW_device.connections[0].frame_out.service = ESPNOW_DEVICE_SERVICE_SHELL;
  char *txt = (char*) ESPNOW_device.connections[0].frame_out.body;
  strncpy( txt, input, 200 );
  ESPNOW_device.connections[0].frame_out.len = strlen(txt);
  ESPNOW_device.connections[0].send();
}


// ==============================================================================
// espnow device handle
// ==============================================================================

void espnow_handle(espnow_device_event_t ev, int id){
  
  if( ev == ESPNOW_EVT_NOTIFY ){

    for( int i=0;i<ESPNOW_device.connections_made();i++){
      ESPNOW_device.connections[i].frame_out.service = ESPNOW_DEVICE_SERVICE_NONE;
    }
    
  }else if( ev == ESPNOW_EVT_PUBLIC_NOTIFY ){
    
    Serial.printf( "[PUBLIC NOTIFY %d]\n", id );
    for( int i=0;i<ESPNOW_device.connections_made();i++){
      ESPNOW_device.connections[i].frame_out.service = ESPNOW_DEVICE_SERVICE_NONE;
    }

  }else if( ev == ESPNOW_EVT_DISCONNECTED ){
    Serial.printf( "[FALL %d]\n", id );

  }else if( ev == ESPNOW_EVT_CONNECTED ){
    Serial.printf( "[RISE %d]\n", id );

  }else if( ev == ESPNOW_EVT_RECIVE ){

    // Terminal command
    if( ESPNOW_device.connections[0].frame_in.service == ESPNOW_DEVICE_SERVICE_SHELL ){
      String msg = "";
      msg.concat(
        (const char*) ESPNOW_device.connections[0].frame_in.body,
        ESPNOW_device.connections[0].frame_in.len
      );
      Serial.printf( "[RECIVE -> %s] ", ( id >= 0 ? ESPNOW_device.connections[id].remote_name : "broadcast" ) );
      printf("> %s\n", msg.c_str() );
    }

  }
}

void setup() {

  // inicia o LED
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);

  // Serial
  Serial.begin(115200);
  Serial.setTimeout(20);

  ESPNOW_device.set_led( 2 );
  ESPNOW_device.set_handle_function( espnow_handle );
  ESPNOW_device.connections[0].waiting_ms_disconnect = 1000;

  // ESPNOW_device
  Serial.println( "======== ESPNOW Device Client ========" );
  ESPNOW_device.begin_client( "RADIO" );
  ESPNOW_device.connect( "ROBOT", "1324" );
  ESPNOW_device.connections[0].delay_ms_notify = 50;

}

void loop() {
  
  ESPNOW_device.update();

  if(Serial.available() > 0){
    String CMD = Serial.readString();
    Serial.println(CMD);
    for( int i=0;i<ESPNOW_device.connections_made();i++){
      if( CMD.startsWith( ESPNOW_device.connections[i].remote_name ) ){
        CMD.remove( 0, strlen( ESPNOW_device.connections[i].remote_name ) );
        espnow_device_print( CMD.c_str() );
      }
    }
  }

}
