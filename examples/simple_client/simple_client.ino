#include "ESPNOW_device.h"

#define LED 2
#define LED_ON 0

void handle_espnow(int event){
  
  if( event == ESPNOW_device.EVT_RECIVE ){
    //int len = ESPNOW_device.pass_service( ESPNOW_DEVICE_SERVICE__RADIO_CHANNELS, CH, sizeof(CH) );
    //if( len > 0 ){
    //  Serial.printf( ">>" );
    //  for(int i=0;i<len;i++) Serial.printf( "%d\t", CH[i] );
    //  Serial.println();
    //  motor.diff_drive(CH[0], CH[1], false, true);
    //}
  }else if( event == ESPNOW_device.EVT_RISE_CONNECTION ){
    Serial.println("[RISE]");
    digitalWrite( LED, LED_ON );
  }else if( event == ESPNOW_device.EVT_FALL_CONNECTION ){
    digitalWrite( LED, !LED_ON );
    Serial.println("[FALL]");
  }else if( event == ESPNOW_device.EVT_SEND ){
    //battery.cells = 2;
    //battery.voltage = 7.5;
    //Serial.println( ESPNOW_device.add_service(  ESPNOW_DEVICE_SERVICE__BATTERY_MONITOR, (uint8_t*) &battery, sizeof(battery) ) ? "-> PASS OK!" : "-> PASS Fail!" );
    //ESPNOW_device.list_services( &ESPNOW_device.data_out );
  }else if( event == ESPNOW_device.EVT_LOST_CONNECTION ){
    Serial.println("[ LOST CONNECTION!! ]");
  }

}

void setup() {

  // begin Serial
  Serial.begin(115200);

  // begin ESPNOW_device
  ESPNOW_device.beginClient( "Radio" );
  ESPNOW_device.WriteDelay = 200;
  ESPNOW_device.offline_timeout = 500;
  ESPNOW_device.disconnect_timeout = 800;
  ESPNOW_device.set_handle_function(handle_espnow);
  ESPNOW_device.connect("ninja");

  // LED
  pinMode(LED,OUTPUT);
  digitalWrite( LED, !LED_ON );

}

void loop() {
  
  ESPNOW_device.update();

}
