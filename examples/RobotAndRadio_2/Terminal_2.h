#ifndef TERMINAL_H
#define TERMINAL_H

#include "Arduino.h"

/////////////////////////////////////////////////////////////////////////////////
///// Funções basicas do terminal ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

boolean extract_float( String * str, float *x ){
  str->trim();
  char *ptr;
  *x = strtof(str->c_str(), &ptr);
  if( abs(*x) > 0 || str->indexOf('0') == 0 || str->indexOf("-0") == 0 ){
    str->remove( 0, ptr - str->c_str() );
    return true;
  }
  return false;
}

boolean extract_int( String * str, int *x ){
  str->trim();
  char *ptr;
  *x = strtol(str->c_str(), &ptr, 10);
  if( abs(*x) > 0 || str->indexOf('0') == 0 || str->indexOf("-0") == 0 ){
    str->remove( 0, ptr - str->c_str() );
    return true;
  }
  return false;
}

boolean extract_key( String * str, const char * _key ){
  String key = _key;
  str->trim();
  uint32_t key_len = key.length();
  if( str->indexOf( key ) == 0 ){
    if( str->length() == key_len    ){ *str = "";              return true; }
    if( str->charAt(key_len) == ' ' ){ str->remove(0,key_len); return true; }
  }
  return false;
}


#endif