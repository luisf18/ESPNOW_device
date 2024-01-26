#include "ESPNOWSerial.h"

#define TTGO

#ifdef TTGO
//#include "TTGO_T1.h"
#include "M5StickCPlus.h"
#define T1 M5
#endif

#include "ESPNOW_device.h"
#include "ESPNOW_device_defines.h"

#include "Terminal_local.h"

void handle_espnow(int event){
  
  if( event == ESPNOW_device.EVT_RECIVE ){
    uint16_t ch[20];
    int len = ESPNOW_device.pass_service( ESPNOW_DEVICE_SERVICE__RADIO_CHANNELS, ch, sizeof(ch) );

    if( len > 0 ){
      T1.Lcd.setTextColor(GREEN,BLACK);
      T1.Lcd.setCursor(110, 80); T1.Lcd.printf( "ch_len: %i ", len );
      T1.Lcd.setCursor(110,100); T1.Lcd.printf( "%i ", ch[0] );
      T1.Lcd.setCursor(110,120); T1.Lcd.printf( "%i ", ch[1] );
    }

    uint8_t level;
    len = ESPNOW_device.pass_service( ESPNOW_DEVICE_SERVICE__BATTERY_MONITOR, &level, 1 );
    if( len > 0 ){
      T1.Lcd.setTextColor(WHITE);
      T1.Lcd.fillCircle( 210,25,28,BLUE);
      T1.Lcd.setCursor(200,18); T1.Lcd.printf( "%i%%", level );
    }
  }else if( event == ESPNOW_device.EVT_RISE_CONNECTION ){
    Serial.println("[RISE]");
    #ifndef TTGO
    digitalWrite(2,HIGH);
    #else
    T1.Lcd.setTextColor(GREEN,BLACK);
    T1.Lcd.setCursor(10,100);
    T1.Lcd.println("ONLINE ");
    #endif
  }else if( event == ESPNOW_device.EVT_FALL_CONNECTION ){
    Serial.println("[FALL]");
    #ifndef TTGO
    digitalWrite(2,LOW);
    #else
    T1.Lcd.setTextColor(RED,BLACK);
    T1.Lcd.setCursor(10,100);
    T1.Lcd.println("OFFLINE");
    #endif
  }

}




void setup() {

  // Serial
  Serial.begin(115200);
  Serial.setTimeout(20);

  // Terminal
  terminal_command_list_add( terminal_espnow_device );
  terminal_command_list_add( terminal_display );

  #ifdef TTGO

  // ESPNOW
  ESPNOW_device.beginServer("ninja");
  ESPNOW_device.WriteDelay = 1000;
  ESPNOW_device.LostConnectionDelay = 500;
  ESPNOW_device.set_mode(ESPNOW_device.CONNECTING);
  //ESPNOW_device.set_recive_callback(cb_recive);
  ESPNOW_device.set_handle_function(handle_espnow);

  // Display
  T1.begin();
  T1.Lcd.setTextColor(YELLOW);
  T1.Lcd.setTextSize(2);
  T1.Lcd.setRotation(3);
  T1.Lcd.println("Dispositivo on!");
  //T1.Lcd.setTextColor(RED);
  
  #else
  pinMode(2,OUTPUT);
  #endif

}

uint8_t radio_mode_last = ESPNOW_device.OFF;

void loop() {
  
  Terminal_loop();

  ESPNOW_device.update();

  #ifdef TTGO
  if( ESPNOW_device.mode != radio_mode_last ){
    T1.Lcd.fillScreen(BLACK);
    T1.Lcd.setCursor(0,10);
    
    T1.Lcd.setTextColor(YELLOW);
    T1.Lcd.print( ESPNOW_device.is_server ? " Server: " : " Client: ");
    T1.Lcd.setTextColor(ORANGE);
    T1.Lcd.println(ESPNOW_device.name);
    //T1.Lcd.println(ESPNOWSerial.mac2str(ESPNOW_device.link_mac));

    if( ESPNOW_device.mode == ESPNOW_device.SCAN ){
      T1.Lcd.setTextColor(PINK);
      T1.Lcd.println("> Scanning...");
    }else if( ESPNOW_device.mode == ESPNOW_device.CONNECTING ){
      T1.Lcd.setTextColor(BLUE);
      T1.Lcd.println("> Connecting...");
    }if( ESPNOW_device.mode == ESPNOW_device.CONNECTED ){
      T1.Lcd.setTextColor(GREEN);
      T1.Lcd.println("> Connected!");
      T1.Lcd.setTextColor(BLUE);
      if( ESPNOW_device.is_server ){
        T1.Lcd.print("Server: ");
        T1.Lcd.println(ESPNOW_device.connected_name);
        T1.Lcd.println(ESPNOWSerial.mac2str(ESPNOW_device.connected_mac));
      }
    }
    radio_mode_last = ESPNOW_device.mode;
  }
  #endif

}
