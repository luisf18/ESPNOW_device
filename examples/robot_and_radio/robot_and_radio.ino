#include "ESPNOW_device.h"

// comment to upload to radio
#define ROBOT

#ifdef ROBOT
  /*  Motores  */
  #include "DRV8833.h"
  #define PIN__MOTOR_IN1 18
  #define PIN__MOTOR_IN2 19
  #define PIN__MOTOR_IN3 4
  #define PIN__MOTOR_IN4 23
  DRV8833   motor  = DRV8833(
    PIN__MOTOR_IN1,
    PIN__MOTOR_IN2,
    PIN__MOTOR_IN3,
    PIN__MOTOR_IN4
  );
#endif

typedef struct{
  uint8_t cells   = 2;
  float   voltage = 0;
  uint8_t level   = 0;
} notify_t;

enum ESPNOW_device_service{
   ESPNOW_DEVICE_CHANNEL_SERVICE = 18,
   ESPNOW_DEVICE_NOTIFY_SERVICE
};

void espnow_handle(espnow_device_event_t ev, int id){
  
  if( ev == ESPNOW_EVT_NOTIFY ){
    
    Serial.printf( "[NOTIFY %d]\n", id );

    #ifdef ROBOT
      ESPNOW_device.connections[0].frame_out.service = ESPNOW_DEVICE_NOTIFY_SERVICE;
      ESPNOW_device.connections[0].frame_out.len = sizeof( notify_t );
      notify_t *notify_pack = (notify_t*) ESPNOW_device.connections[0].frame_out.body;
      notify_pack->cells   = 2;
      notify_pack->voltage = ((28.186 - 0.886)/4095.0)*(float)analogRead( 36 ) + 0.886;
      notify_pack->level   = constrain( 100*( notify_pack->voltage/(float)notify_pack->cells - 3.6 )/(4.2-3.6), 0, 100 );
    #else
      ESPNOW_device.connections[0].frame_out.service = ESPNOW_DEVICE_CHANNEL_SERVICE;
      ESPNOW_device.connections[0].frame_out.len = sizeof(int)*2;
      int *ch = (int*) ESPNOW_device.connections[0].frame_out.body;
      ch[0] = 1500;
      ch[1] = ( digitalRead(0) ? 1500 : 2000 );
    #endif

  }else if( ev == ESPNOW_EVT_PUBLIC_NOTIFY ){
    Serial.printf( "[PUBLIC NOTIFY %d]\n", id );
  }else if( ev == ESPNOW_EVT_DISCONNECTED ){
    Serial.printf( "[FALL %d]\n", id );
    #ifdef ROBOT
      motor.bip(5,30,4000);
      motor.stop();
    #endif
  }else if( ev == ESPNOW_EVT_CONNECTED ){
    Serial.printf( "[RISE %d]\n", id );
    #ifdef ROBOT
      motor.bip(3,20,2000);
    #endif
  }else if( ev == ESPNOW_EVT_RECIVE ){
    Serial.printf( "[RECIVE %d]\n", id );
    #ifdef ROBOT
      if( ESPNOW_device.connections[0].frame_in.service == ESPNOW_DEVICE_CHANNEL_SERVICE && ESPNOW_device.connections[0].frame_out.len >= sizeof(int)*2 ){
        int *CH = (int*) ESPNOW_device.connections[0].frame_in.body;
        motor.diff_drive(CH[0], CH[1], false, true);
        Serial.printf( "CH: %d\t%d\n\n", CH[0], CH[1] );
      }
    #endif
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
  ESPNOW_device.connections[0].waiting_ms_disconnect = 500;

  #ifdef ROBOT
    // ESPNOW_device
    Serial.println( "======== ESPNOW Device Server ========" );
    ESPNOW_device.set_led( 2 );
    ESPNOW_device.set_handle_function( espnow_handle );
    ESPNOW_device.begin_server( "ROBOT", "1324" );
    ESPNOW_device.connections[0].delay_ms_notify = 200;
    // Begin Motors
    motor.begin();
    motor.sound_vol(12);
  #else
  
  // ESPNOW_device
  Serial.println( "======== ESPNOW Device Client ========" );
  ESPNOW_device.begin_client( "RADIO" );
  ESPNOW_device.connect( "ROBOT", "1324" );
  ESPNOW_device.connections[0].delay_ms_notify = 50;

  #endif

}

void loop() {
  
  ESPNOW_device.update();

}
