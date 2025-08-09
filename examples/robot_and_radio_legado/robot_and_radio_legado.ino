#include "ESPNOW_device.h"

//#define POINTER_TO_U32( p ) ( (uint32_t) p )

#define SERVER

void espnow_handler( espnow_device_event_t evt, ESPNOW_connection *cn ){
  switch( evt ){
    case ESPNOW_DEVICE__EVT_NULL:   Serial.println("[NULL]");   break;

    case ESPNOW_DEVICE__EVT_NOTIFY: Serial.println("[NOTIFY]"); break;
    
    case ESPNOW_DEVICE__EVT_SEND:
      Serial.println("[SEND]");
      // init
      ESPNOW_device.frame.service = ESPNOW_SERVICE::RADIO;
      ESPNOW_device.frame.len     = sizeof( ESPNOW_SERVICE::radio_t );
      
      // Battery
      ESPNOW_device.frame.radio.battery.id = ESPNOW_SERVICE::battery_t::LiPo;
      ESPNOW_device.frame.radio.battery.cells   = 2;
      ESPNOW_device.frame.radio.battery.level   = 100;
      ESPNOW_device.frame.radio.battery.voltage = 10;
      ESPNOW_device.frame.radio.battery.current = 0.1;
      ESPNOW_device.frame.radio.battery.temp    = 30;
      // MCU
      ESPNOW_device.frame.radio.mcu.id = ESPNOW_SERVICE::mcu_t::MCU_ESP32;
      ESPNOW_device.frame.radio.mcu.voltage_mV = 3300;
      ESPNOW_device.frame.radio.mcu.current_mA = 0;
      ESPNOW_device.frame.radio.mcu.temp       = 30;
    break;

    case ESPNOW_DEVICE__EVT_RECIVE:
      Serial.printf("[RECIVE][ %s ][ %d | %d bytes ]\n",cn->name,cn->frame.service,cn->frame.len );
      if( cn->frame.service == ESPNOW_SERVICE::RADIO_LEGACY ){
        //int *CH = (int*) cn->frame.body;
        Serial.print("[ "); for(int i=0;i<8;i++) Serial.printf("%d ", cn->frame.ch[i] ); Serial.println("]");
        //Serial.print("[ "); for(int i=0;i<8;i++) Serial.printf("%d ", CH[i] ); Serial.println("]");
        //Serial.print("[ "); for(int i=0;i<8;i++) Serial.printf("%02X ", cn->frame.body[i] ); Serial.println("]");
      }else{
        Serial.printf("[Serviço desconhecido ou inesperado]\n" );
      }
    break;
    case ESPNOW_DEVICE__EVT_CONNECTED:    Serial.printf("[CONNECTED][ %s ]\n",cn->name);    break;
    case ESPNOW_DEVICE__EVT_DISCONNECTED: Serial.printf("[DISCONNECTED][ %s ]\n",cn->name); break;
    case ESPNOW_DEVICE__EVT_SCAN_FOUND:
      Serial.printf("[Found][ %s - %s ]\n", cn->name, mac2str( cn->mac ).c_str() );
    break;
  }
}

void setup() {
  Serial.begin(115200);

  #ifdef SERVER
    ESPNOW_device.set_handler( espnow_handler );
    ESPNOW_device.begin_server("ROBOT","1234",1);
    ESPNOW_device.frame.radio.init( ESPNOW_SERVICE::devices::ROBOT_DIF_2W );
  #else
    ESPNOW_device.set_handler( espnow_handler );
    ESPNOW_device.begin_client("ESPNOW_client",1);
    ESPNOW_device.add_server( "ESPNOW_Server", "1234" );
    ESPNOW_device.add_server( "ESPNOW_Server2", "4321" );
    ESPNOW_device.frame.radio.init( 0 );
  #endif
}

void loop() {
  #ifdef SERVER
  ESPNOW_device.loop();
  #endif
}
