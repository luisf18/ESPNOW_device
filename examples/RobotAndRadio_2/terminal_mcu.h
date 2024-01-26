#include "Terminal.h"

/////////////////////////////////////////////////////////////////////////////////
///// MCU ///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

String terminal_mcu( String *msg ){

  String answer;
  
  boolean ok = false;
  
  if( extract_key(msg,"pinmode") || extract_key(msg,"mode") ){
    
    String str_mode = "";
    int pin, mode;
    
    if( extract_int(msg,&pin) ){ // pin
            if( extract_key(msg,"input"         ) || extract_key(msg,"in"    ) ){ ok = true;  mode = INPUT;          str_mode = "INPUT"; }
      else if( extract_key(msg,"input_pullup"  ) || extract_key(msg,"in_up"  ) ){ ok = true;  mode = INPUT_PULLUP;   str_mode = "INPUT_PULLUP"; }
      else if( extract_key(msg,"input_pulldown") || extract_key(msg,"in_down") ){ ok = true;  mode = INPUT_PULLDOWN; str_mode = "INPUT_PULLDOWN"; }
      else if( extract_key(msg,"output")         || extract_key(msg,"out"    ) ){ ok = true;  mode = OUTPUT;         str_mode = "OUTPUT";       }
    }

    answer += "mcu pinmode";

    if(ok){
      answer += ( "(" + String( pin ) + "," + str_mode + ")" );
      pinMode(pin,mode);
    }else{
      answer += " erro / modos: { in, in_up, in_down, out }";
    }

  }else if( extract_key(msg,"digitalwrite") || extract_key(msg,"write") ){
    
    int pin, val;
    
    if( extract_int(msg,&pin) ){
      if( extract_int(msg,&val) ) ok = true;
    }

    answer += "digitalWrite";

    if(ok){
      answer += ( "(" + String( pin ) + "," + String( val ) + ")" );
      digitalWrite( pin, val );
    }else{
      answer += " erro";
    }
    
  }else if( extract_key(msg,"analogread") || extract_key(msg,"anread") ){
    
    int pin;
    
    if( extract_int(msg,&pin) ) ok = true;

    answer += "analogRead";

    if(ok){
      answer += ( "(" + String( pin ) + ") = " + String( analogRead( pin ) ) );
    }else{
      answer += " erro";
    }

  }else if( extract_key(msg,"digitalread") || extract_key(msg,"read") ){
    
    int pin;
    
    if( extract_int(msg,&pin) ) ok = true;

    answer += "digitalRead";

    if(ok){
      answer += ( "(" + String( pin ) + ") = " + String( digitalRead( pin ) ) );
    }else{
      answer += " erro";
    }
    
  }else if( extract_key(msg,"reset") ){

    Serial.println( "[RESET] reiniciando via softeare..." );
    ESP.restart();

  }

  return answer;

}