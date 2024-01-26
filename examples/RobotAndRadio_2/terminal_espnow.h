#include "ESPNOWSerial.h"
#include "ESPNOW_device.h"

String terminal_radio( String *msg ){
  
  String answer;
  boolean ok = false;

  if( extract_key(msg,"begin") ){
    answer += "begin: ";
    
    boolean is_server = false;
         if( extract_key(msg,"client") ){ answer += "client"; is_server = false; }
    else if( extract_key(msg,"server") ){ answer += "server"; is_server = true; }
    else                                { answer += "fail"; return answer; }
    msg->trim();
    if( msg->length() > 1 ){
      ok = true;
      radio.begin( *msg, is_server );
      
      radio.WriteDelay = 500;
      radio.LostConnectionDelay = 1000;

      answer += ( " name: " + *msg );
    }

  }else if( extract_key(msg,"connect") ){
    answer += "connect: ";

    msg->trim();
    if( msg->length() > 1 ){
      ok = true;
      radio.connect( *msg );
      String name = radio.connected_name;
      answer += ( " name: " + name );
    }

  }else if( extract_key(msg,"mode") ){
    ok = true;
    answer += "mode: ";
         if( extract_key(msg,"off"       ) ) radio.set_mode(radio.OFF       );
    else if( extract_key(msg,"scan"      ) ) radio.set_mode(radio.SCAN      );
    else if( extract_key(msg,"connecting") ) radio.set_mode(radio.CONNECTING);
    else if( extract_key(msg,"connected" ) ) radio.set_mode(radio.CONNECTED );
         if( radio.mode == radio.OFF        ) answer += "OFF";
    else if( radio.mode == radio.SCAN       ) answer += "SCAN";
    else if( radio.mode == radio.CONNECTING ) answer += "CONNECTING";
    else if( radio.mode == radio.CONNECTED  ) answer += "CONNECTED";
  }else if( extract_key(msg,"is_server") ){
    ok = true;
    answer += "device is ";
    answer += ( radio.is_server ? "server" : "client" );
  }else if( extract_key(msg,"send") ){
    ok = true;
    answer += "send: LOAD";
    
    uint8_t data[4] = {1,2,3,4};
    radio.add_service( 0xF000, data, sizeof(data) );
    int16_t channels[2] = {2000,1500};
    radio.add_service( 0xF100, (uint8_t*) channels, sizeof(channels) );
    uint8_t battery = 85;
    radio.add_service( 0xF200, &battery, sizeof(battery) );

    radio.send();
  }else{
    
    uint8_t type = 0;

         if( extract_key(msg,"send_delay"  ) ){ answer += "send delay: ";   type = 1; }
    else if( extract_key(msg,"recive_delay") ){ answer += "recive delay: "; type = 2; }

    if( type > 0 ){
      int dt = 0;
      if( extract_int(msg,&dt) ){
        if( dt >= 0 ){
          ok = true;
               if(type == 1) radio.WriteDelay = dt;
          else if(type == 2) radio.LostConnectionDelay = dt;
          answer += String(dt) + " ms";
        }
      }else{
        ok = true;
             if(type == 1) dt = radio.WriteDelay;
        else if(type == 2) dt = radio.LostConnectionDelay;
        answer += String(dt) + " ms";
      }
    }

  }

  if( !ok ) answer += "fail";

  return answer;
}