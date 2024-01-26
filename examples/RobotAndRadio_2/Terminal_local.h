#include "Terminal.h"

#define Display T1.Lcd

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
      resp = terminal_execute_variable(in, out, mode, "delay_send",     &ESPNOW_device.WriteDelay          ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable(in, out, mode, "delay_recive",   &ESPNOW_device.LostConnectionDelay ); COMMAND_LIST_CHECK(resp)
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

/*/
else if( extract_key(msg,"connect") ){
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

         if( radio.mode == radio.OFF        ) answer += "OFF";
    else if( radio.mode == radio.SCAN       ) answer += "SCAN";
    else if( radio.mode == radio.CONNECTING ) answer += "CONNECTING";
    else if( radio.mode == radio.CONNECTED  ) answer += "CONNECTED";
  }

/*/


#ifdef Display
// DISPLAY
int terminal_display(String * in, String * out, int mode){
    int resp = (mode == TERMINAL_MODE_HELP ? TERMINAL_RETURN_HELP : TERMINAL_RETURN_NOT_FOUND);
    
    // espnow.x
    int r = terminal_execute_class(in, out, mode, "display");
    if (r == 2){
      
      if (mode == TERMINAL_MODE_HELP) *out += "\tMethods:\n";
      TS_CALLER( "setCursor", { T_VAR(uint16_t,x) T_VAR(uint16_t,y) Display.setCursor(x,y); T_RESULT_VOID() } )
      TS_CALLER( "fillScreen", { T_VAR(uint32_t,color)  Display.fillScreen(color); T_RESULT_VOID() } )
      TS_CALLER( "setTextSize", { T_VAR(uint8_t ,size)  Display.setTextSize(size); T_RESULT_VOID() } )
      TS_CALLER( "setTextFont", { T_VAR(uint8_t ,font)  Display.setTextFont(font); T_RESULT_VOID() } )
      TS_CALLER( "fontsLoaded", { T_RESULT( T1.Lcd.fontsLoaded() ) } )

      TS_CALLER( "setTextColor", {
        T_VAR(uint32_t,color)
        uint32_t bgcolor;
        if( terminal_recursive_get_var( in, out, TERMINAL_MODE_GET, &bgcolor ) ) Display.setTextColor(color,bgcolor);
        else Display.setTextColor(color);
        T_RESULT_VOID()
      } )

      TS_CALLER( "fontHeight", {
        int16_t font;
        if( terminal_recursive_get_var( in, out, TERMINAL_MODE_GET, &font ) ){ T_RESULT( Display.fontHeight(font) ) }
        else{ T_RESULT( Display.fontHeight() ) }
      } )

      TS_CALLER( "print", {
        String txt;
        if( terminal_recursive_get_var( in, out, TERMINAL_MODE_GET, &txt ) ) Display.print(txt);
        else Display.print(*in);
        T_RESULT_VOID()
      } )
      
      TS_CALLER( "println", {
        String txt;
        if( terminal_recursive_get_var( in, out, TERMINAL_MODE_GET, &txt ) ) Display.println(txt);
        else Display.println(*in);
        T_RESULT_VOID()
      } )

      TS_CALLER( "begin",         { Display.begin(); T_RESULT_VOID() } )
      TS_CALLER( "setRotation",   { T_VAR(uint8_t,r) Display.setRotation(r); T_RESULT_VOID() } )
      //TS_CALLER( "setBrightness", { T_VAR(uint8_t,brightness) Display.setBrightness(brightness); T_RESULT_VOID() } )
      TS_CALLER( "drawPixel",     { T_VAR(int32_t,x )T_VAR(int32_t,y )T_VAR(uint32_t,color) Display.drawPixel(x,y,color); T_RESULT_VOID() } )
      TS_CALLER( "drawLine",      { T_VAR(int32_t,x0)T_VAR(int32_t,y0)T_VAR(int32_t,xf)T_VAR(int32_t,yf)T_VAR(uint32_t,color) Display.drawLine(x0,y0,xf,yf,color); T_RESULT_VOID() } )
      TS_CALLER( "fillTriangle",  { T_VAR(int32_t,x0)T_VAR(int32_t,y0)T_VAR(int32_t,x1)T_VAR(int32_t,y1)T_VAR(int32_t,x2)T_VAR(int32_t,y2)T_VAR(uint32_t,color) Display.fillTriangle(x0,y0,x1,y1,x2,y2,color); T_RESULT_VOID() } )
      TS_CALLER( "height",        { T_RESULT( T1.Lcd.height() ) } )
      TS_CALLER( "width",         { T_RESULT( T1.Lcd.width()  ) } )

      TS_CALLER( "drawRect", {
        T_VAR(int32_t,x)
        T_VAR(int32_t,y)
        T_VAR(int32_t,w)
        T_VAR(int32_t,h)
        T_VAR(uint32_t,color)
        Display.drawRect(x,y,w,h,color);
        T_RESULT_VOID()
      } )

      TS_CALLER( "fillRect", {
        T_VAR(int32_t,x)
        T_VAR(int32_t,y)
        T_VAR(int32_t,w)
        T_VAR(int32_t,h)
        T_VAR(uint32_t,color)
        Display.fillRect(x,y,w,h,color);
        T_RESULT_VOID()
      } )

      TS_CALLER( "drawCircle", {
        T_VAR(int32_t,x0)
        T_VAR(int32_t,y0)
        T_VAR(int32_t,r)
        T_VAR(uint32_t,color)
        Display.drawCircle(x0,y0,r,color);
        T_RESULT_VOID()
      } )

      TS_CALLER( "fillCircle", {
        T_VAR(int32_t,x0)
        T_VAR(int32_t,y0)
        T_VAR(int32_t,r)
        T_VAR(uint32_t,color)
        Display.fillCircle(x0,y0,r,color);
        T_RESULT_VOID()
      } )


      
      if (mode == TERMINAL_MODE_HELP) *out += "\tVariables:\n";
      resp = terminal_execute_variable(in, out, mode, "rotation",     &Display.rotation ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable(in, out, mode, "textfont",     &Display.textfont ); COMMAND_LIST_CHECK(resp)
      resp = terminal_execute_variable(in, out, mode, "textsize",     &Display.textsize ); COMMAND_LIST_CHECK(resp)
      
    }

    if (mode == TERMINAL_MODE_HELP) *out += "\tdisplay colors:\n";
    resp = terminal_execute_define(in, out, mode, "BLACK",  BLACK   ); COMMAND_LIST_CHECK(resp)
    resp = terminal_execute_define(in, out, mode, "WHITE",  WHITE   ); COMMAND_LIST_CHECK(resp)
    resp = terminal_execute_define(in, out, mode, "BLUE",   BLUE    ); COMMAND_LIST_CHECK(resp)
    resp = terminal_execute_define(in, out, mode, "YELLOW", YELLOW  ); COMMAND_LIST_CHECK(resp)
    resp = terminal_execute_define(in, out, mode, "PINK",   PINK    ); COMMAND_LIST_CHECK(resp)
    //resp = terminal_execute_define(in, out, mode, "GRAY",   GRAY    ); COMMAND_LIST_CHECK(resp)
    resp = terminal_execute_define(in, out, mode, "RED",    RED     ); COMMAND_LIST_CHECK(resp)
    resp = terminal_execute_define(in, out, mode, "GREEN",  GREEN   ); COMMAND_LIST_CHECK(resp)

    return resp;
}
#endif