/*/
UPLOAD Date:
Oct  8 2023
UPLOAD Time:
15:09:24
Running sketch:
bots_vsss.ino
/*/

#include "ESPNOW_device.h"

////////////////////////////////////////////////////////////
// Motor                                                  //
////////////////////////////////////////////////////////////
#include "DRV8833.h"
DRV8833 motor( 12,13,4,5 );

////////////////////////////////////////////////////////////
// LED                                                    //
////////////////////////////////////////////////////////////
class LED{
  private:
  bool EN = false;
  int  Pin = -1;
  bool State_on = HIGH;
  bool Blink = false;
  uint32_t timeout = 0;
  uint32_t Dt;
  public:
  void begin(int pin, bool state_on){
    if(pin>=0){
      EN = true;
      Pin = pin;
      State_on = state_on;
      pinMode(Pin,OUTPUT);
      digitalWrite(Pin,!State_on);
    }
  }
  void on(){  Blink = false; digitalWrite(Pin,State_on);  }
  void off(){ Blink = false; digitalWrite(Pin,!State_on); }
  void blink( uint32_t dt ){
    if(!EN) return;
    Blink = true;
    Dt = dt;
  }
  void update(){
    if(!Blink) return;
    if( timeout <= millis() ){
      timeout = millis() + Dt;
      digitalWrite( Pin, !digitalRead(Pin) );
    }
  }
};

LED led;


float fmap( float x, float in_m, float in_M, float out_m, float out_M ){
  return ( in_m == in_M ? out_m : (x-in_m)*(out_M-out_m)/(in_M-in_m)+out_m );
}

battery_monitor_t battery;

// Battery voltage
float getBatteryVoltage(){
  #ifdef ESP8266
  return fmap(analogRead(A0),0,1023,-0.0255,21.3);
  #else
  return analogRead(34)*(6.6/4095.0);
  #endif
}

// read channels
int16_t CH[20];

#include "Terminal_local.h"

void handle_espnow(int event){
  
  if( event == ESPNOW_device.EVT_RECIVE ){
    int len = ESPNOW_device.pass_service( ESPNOW_DEVICE_SERVICE__RADIO_CHANNELS, CH, sizeof(CH) );
    if( len > 0 ){
      Serial.printf( ">>" );
      for(int i=0;i<len;i++) Serial.printf( "%d\t", CH[i] );
      Serial.println();
      motor.diff_drive(CH[0], CH[1], false, true);
    }
  }else if( event == ESPNOW_device.EVT_RISE_CONNECTION ){
    Serial.println("[RISE]");
    led.on();
  }else if( event == ESPNOW_device.EVT_FALL_CONNECTION ){
    motor.stop();
    led.off();
    Serial.println("[FALL]");
    motor.bip(3,100,3000);
  }else if( event == ESPNOW_device.EVT_SEND ){
    battery.cells = 2;
    battery.voltage = getBatteryVoltage();

    // update led
    if( battery.voltage < 6.6 ) led.blink( ESPNOW_device.connected() ? 80 : 400 );
    else if(ESPNOW_device.online()) led.on();
    else led.off();

    ESPNOW_device.add_service(  ESPNOW_DEVICE_SERVICE__BATTERY_MONITOR, (uint8_t*) &battery, sizeof(battery) );
  }else if( event == ESPNOW_device.EVT_LOST_CONNECTION ){
    Serial.println("[ LOST CONNECTION!! ]");
  }

}



void setup() {

  // Serial
  Serial.begin(115200);
  Serial.setTimeout(20);

  // upload info ////////////////////////////
  Serial.println("\n\n\n\nUPLOAD Date:");
  Serial.println(__DATE__);
  Serial.println("UPLOAD Time:");
  Serial.println(__TIME__);
  Serial.println("Running sketch:");
  Serial.println(__FILE__);
  Serial.println("\n\n");

  // Terminal
  terminal_command_list_add( terminal_espnow_device );
  terminal_command_list_add( terminal_bot );

  // ESPNOW
  ESPNOW_device.beginServer( "ninja" );
  ESPNOW_device.WriteDelay = 200;
  ESPNOW_device.offline_timeout = 500;
  ESPNOW_device.disconnect_timeout = 300;
  ESPNOW_device.set_handle_function(handle_espnow);
  ESPNOW_device.set_mode( ESPNOW_device.CONNECTING );
  //ESPNOW_device.connect( config.devices[ config.sel_device ].name );

  // LED
  led.begin(2,LOW);

  motor.begin();
  motor.bip(2,200,2000);
  
}

void loop() {
  
  Terminal_loop();
  ESPNOW_device.update();
  led.update();

}

