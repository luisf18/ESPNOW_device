#include "Terminal.h"

//struct terminal_dict_element_t <int,String> espnow_modes[2] = { { .key = 0, .value = "FALSE" }, { .key = 1, .value = "TRUE" } };

String espnow_modes_str[2] = {
  "NAO",
  "SIM"
};

String terminal_enum( int key, String *value_list, uint32_t len ){
  return ( key < len ? value_list[key] : "?" );
}
/*/
String terminal_enum( String R, int key, int *key_list, const char * value_list, uint32_t len ){
  for(int i=0;i<len;i++){
    if(key == key_list[i]){
      R = value_list[i];
      break;
    }
  }
  return R;
}
/*/

// espnow
int terminal_espnow_device(String * in, String * out, int mode){
    int resp = (mode == TERMINAL_MODE_HELP ? TERMINAL_RETURN_HELP : TERMINAL_RETURN_NOT_FOUND);
    
    // espnow.x
    int r = terminal_execute_class(in, out, mode, "espnow");
    if (r == 2){
      
      if (mode == TERMINAL_MODE_HELP) *out += "\tMethods:\n";
      TS_CALLER( "begin", { T_VAR(String,name) T_VAR(bool,is_server) T_RESULT_VOID(ESPNOW_device.begin(name,is_server);) } )
      TS_CALLER( "connect", { T_VAR(String,name) T_RESULT_VOID( ESPNOW_device.connect(name); ) } )
      
      TS_CALLER( "send", ARGS({
        uint8_t data[4] = {1,2,3,4};
        ESPNOW_device.add_service( 0xF000, data, sizeof(data) );
        int16_t channels[2] = {2000,1500};
        ESPNOW_device.add_service( 0xF100, (uint8_t*) channels, sizeof(channels) );
        uint8_t battery = 85;
        ESPNOW_device.add_service( 0xF200, &battery, sizeof(battery) );
        ESPNOW_device.send();
        T_RESULT_VOID( );
      }) )

      TS_CALLER( "connect", ARGS({
        T_VAR(String,name)
        ESPNOW_device.connect( name.c_str() );
        T_RESULT( ESPNOW_device.connected_name );
      }) )

      TS_CALLER( "scan", ARGS({
        T_VAR(uint32_t,t)
        ESPNOW_device.scan( t );
        T_RESULT_VOID(  );
      }) )

      TS_CALLER( "server", ARGS({
        //String R = "?";
        //terminal_dict_get_value<int,String>((int)ESPNOW_device.is_server,&R,espnow_modes,2);
        T_RESULT( terminal_enum( ESPNOW_device.is_server, espnow_modes_str, 2 ) );
      }) )


      
      if (mode == TERMINAL_MODE_HELP) *out += "\tVariables:\n";
      resp = terminal_execute_variable(in, out, mode, "send_interval",     &ESPNOW_device.WriteDelay          ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable(in, out, mode, "offline_timeout",   &ESPNOW_device.offline_timeout     ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable(in, out, mode, "disconnect_timeout",   &ESPNOW_device.disconnect_timeout     ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable(in, out, mode, "mode",           &ESPNOW_device.mode                ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable_read_only(in, out, mode, "is_server",      &ESPNOW_device.is_server           ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable_read_only(in, out, mode, "connected_name", &ESPNOW_device.connected_name      ); COMMAND_LIST_CHECK(resp)

      if (mode == TERMINAL_MODE_HELP) *out += "\tDefines:\n";
      resp = terminal_execute_define(in, out, mode, "OFF",         ESPNOW_device.OFF         ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_define(in, out, mode, "SCAN",        ESPNOW_device.SCAN        ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_define(in, out, mode, "CONNECTING",  ESPNOW_device.CONNECTING  ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_define(in, out, mode, "CONNECTED",   ESPNOW_device.CONNECTED   ); COMMAND_LIST_CHECK(resp)
      
    }

    //if (mode == TERMINAL_MODE_HELP) *out += "Variables:\n";
    //resp = terminal_execute_variable(in, out, mode, "SAVE",  &terminal_save_cmd); COMMAND_LIST_CHECK(resp)
    //if (mode == TERMINAL_MODE_HELP) *out += "Functions:\n";
    //resp = terminal_execute(in, out, mode, "read",      digitalRead    ); COMMAND_LIST_CHECK(resp)
    
    return resp;
}



// espnow
int terminal_bot(String * in, String * out, int mode){
    int resp = (mode == TERMINAL_MODE_HELP ? TERMINAL_RETURN_HELP : TERMINAL_RETURN_NOT_FOUND);
    
    // espnow.x
    int r = terminal_execute_class(in, out, mode, "bot");
    if (r == 2){
      
      if (mode == TERMINAL_MODE_HELP) *out += "\tMethods:\n";
      TS_CALLER( "bip", {
        //T_VAR(uint8_t,n)
        //T_VAR(uint16_t,dt)
        //T_VAR(uint32_t,tone)
        //motor.bip(n,dt,tone);
        motor.bip(1,200,2000);
        T_RESULT_VOID()
      } )
      
      TS_CALLER( "move", {
        T_VAR(int,speed_0)
        T_VAR(int,speed_1)
        motor.move(speed_0,speed_1);
        T_RESULT_VOID()
      } )

      TS_CALLER( "stop", { motor.stop(); T_RESULT_VOID() } )
      TS_CALLER( "off", { motor.off(); T_RESULT_VOID() } )
      TS_CALLER( "read", { T_VAR(int,n) T_RESULT( motor.read(n) ) } )
      
      if (mode == TERMINAL_MODE_HELP) *out += "\tVariables:\n";
      //resp = terminal_execute_variable(in, out, mode, "send_interval",     &ESPNOW_device.WriteDelay          ); COMMAND_LIST_CHECK(resp)

      if (mode == TERMINAL_MODE_HELP) *out += "\tDefines:\n";
      //resp = terminal_execute_define(in, out, mode, "OFF",         ESPNOW_device.OFF         ); COMMAND_LIST_CHECK(resp)
      
    }

    //if (mode == TERMINAL_MODE_HELP) *out += "Variables:\n";
    //resp = terminal_execute_variable(in, out, mode, "SAVE",  &terminal_save_cmd); COMMAND_LIST_CHECK(resp)
    //if (mode == TERMINAL_MODE_HELP) *out += "Functions:\n";
    //resp = terminal_execute(in, out, mode, "read",      digitalRead    ); COMMAND_LIST_CHECK(resp)
    
    return resp;
}