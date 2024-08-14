#ifndef ESPNOW_LOCAL_H
#define ESPNOW_LOCAL_H

#include "ESPNOW_device.h"

//==========================================================================
// ESPNOW devices
//==========================================================================

enum ESPNOW_device_service{
   ESPNOW_DEVICE_SERVICE_CHANNEL = 18,
   ESPNOW_DEVICE_SERVICE_NOTIFY,
   ESPNOW_DEVICE_SERVICE_SHELL
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

//==========================================================================
// ESPNOW devices handle
//==========================================================================

void espnow_handle(espnow_device_event_t ev, int id){
  
  if( ev == ESPNOW_EVT_NOTIFY ){
    
    //Serial.printf( "[NOTIFY %d]\n", id );
    
    ESPNOW_device.connections[0].frame_out.service = ESPNOW_DEVICE_SERVICE_NOTIFY;
    ESPNOW_device.connections[0].frame_out.len = sizeof( my_notify_t );
    my_notify_t *notify_pack = (my_notify_t*) ESPNOW_device.connections[0].frame_out.body;
    notify_pack->cells   = 2;
    notify_pack->voltage = BAT.voltage();
    notify_pack->level   = constrain( 100*( notify_pack->voltage/(float)notify_pack->cells - 3.6 )/(4.2-3.6), 0, 100 );


  }else if( ev == ESPNOW_EVT_PUBLIC_NOTIFY ){
    
    //Serial.printf( "[PUBLIC NOTIFY %d]\n", id );
    ESPNOW_device.connections[0].frame_out.service = ESPNOW_DEVICE_SERVICE_NOTIFY;
    ESPNOW_device.connections[0].frame_out.len = sizeof( my_notify_t );
    my_notify_t *notify_pack = (my_notify_t*) ESPNOW_device.connections[0].frame_out.body;
    notify_pack->cells   = 2;
    notify_pack->voltage = BAT.voltage();
    notify_pack->level   = constrain( 100*( notify_pack->voltage/(float)notify_pack->cells - 3.6 )/(4.2-3.6), 0, 100 );
  
  }else if( ev == ESPNOW_EVT_DISCONNECTED ){

    Serial.printf( "[FALL %d]\n", id );
    motor.stop();
    motor.bip(5,30,4000);
    motor.stop();

  }else if( ev == ESPNOW_EVT_CONNECTED ){
    
    Serial.printf( "[RISE %d]\n", id );
    motor.bip(3,20,2000);
    espnow_device_print( "hello from server: " );
    espnow_device_print( ESPNOW_DEVICE_NAME );

  }else if( ev == ESPNOW_EVT_RECIVE ){
    
    //Serial.printf( "[RECIVE %d SERVICE: %d ]\n", id, ESPNOW_device.connections[0].frame_in.service );

    // Radio channels
    if( ESPNOW_device.connections[0].frame_in.service == ESPNOW_DEVICE_SERVICE_CHANNEL ){
      if( ESPNOW_device.connections[0].frame_out.len >= sizeof(int32_t)*2 ){
        int32_t *CH = (int32_t*) ESPNOW_device.connections[0].frame_in.body;
        motor.diff_drive(CH[1], CH[0], false, true);
        Serial.printf( "CH: %d\t%d\n\n", CH[0], CH[1] );
      }
    }

    // Terminal command
    else if( ESPNOW_device.connections[0].frame_in.service == ESPNOW_DEVICE_SERVICE_SHELL ){
      String msg = "";
      msg.concat(
        (const char*) ESPNOW_device.connections[0].frame_in.body,
        ESPNOW_device.connections[0].frame_in.len
      );
      printf("CMD> %s\n", msg.c_str() );
      ESPNOW_device_cmd = msg;
    }

  }
}

#endif