// ================================================================================
// TERMINAL
// ================================================================================

template<typename T>
String str_array(const char *nome, T * arr, uint32_t sz){
    String str = String(nome) + " = [ ";
    for (int i=0; i<sz-1; i++) str += ( String(arr[i]) + " " );
    str += String(arr[sz-1]) + " ]";
    return str;
}

String terminal( const char *const cmd ){
    
    // obtem os valores
    char key[20]; float x,y,z;
    int i = sscanf(cmd,"%s %f %f %f ",key,&x,&y,&z);
    if(i==0) return "> fail";

    String resposta = "> ";

    // --------------------------------------------------------------
    // MCU
    // --------------------------------------------------------------
    if (strcmp(key,"reset") == 0 ) {
        ESP.restart();
    }else if ( strcmp(key,"mac") == 0 || strcmp(key,"MAC") == 0 ) {
        resposta += "ESP Board MAC Address:  " + WiFi.macAddress() + "\n";
    }else if ( strcmp(key,"digitalWrite") == 0 ) {
        resposta += "digitalWrite: ";
        if( i > 2 ){
          x = constrain( x, 0, 40 ); // pin
          y = constrain( y, 0, 1  ); // state
          digitalWrite(x,y);
          resposta += "pin " + String( (int)x ) + " -> " + String((int)y) + "\n";
        }else{
          resposta += "fail\n";
        }
    }else if ( strcmp(key,"digitalRead") == 0 ) {
        resposta += "digitalRead: ";
        if( i > 1 ){
          x = constrain( x, 0, 40 ); // pin
          resposta += "pin " + String( (int)x ) + " -> " + String( digitalRead((int)x) ) + "\n";
        }else{
          resposta += "fail\n";
        }
    }
    
    // --------------------------------------------------------------
    // Sensoriamento
    // --------------------------------------------------------------
    else if (strcmp(key,"voltage") == 0 ) {
      resposta += "voltage: " + String(voltage()) + " V\n";
    }else if (strcmp(key,"pixel") == 0 ) {
      uint32_t color = px_color;
      if( i == 4 ){
        x = constrain( x, 0, 255 );
        y = constrain( y, 0, 255 );
        z = constrain( z, 0, 255 );
        color = RGB_24((uint8_t)x,(uint8_t)y,(uint8_t)z);
      }else if( i > 1 ){
        color = (uint32_t) x;
      }
      resposta += "pixel color: " + String( color, HEX ) + " \n";
      px_set_color_raw( color );
      delay(500);
    }

    // --------------------------------------------------------------
    // Help
    // --------------------------------------------------------------
    else if (strcmp(key,"help") == 0 ) {
        resposta += "help list:\n";
        //resposta += "speed_tg:\t" + String(pid_speed  ) + "\n";
        //resposta += "speed_w:\t"  + String(pid_speed_w) + "\n";
        //resposta += "Kp:\t"       + String(PID_w.kp) + "\n";
        //resposta += "Ki:\t"       + String(PID_w.ki) + "\n";
        //resposta += "Kd:\t"       + String(PID_w.kd) + "\n";
        //resposta += "P:\t"        + String(PID_w.P) + "\n";
        //resposta += "I:\t"        + String(PID_w.I) + "\n";
        //resposta += "D:\t"        + String(PID_w.D) + "\n";
        //resposta += "Imax:\t"     + String(PID_w.Imax) + "\n";
        //resposta += "outros comandos:\n";
        resposta += "reset\n";
        resposta += "start\n";
        resposta += "stop\n";
        resposta += "speed\n";
        resposta += "w\n";
        resposta += "help\n";
    }
    
    else if (strcmp(key,"info") == 0 ) {
      resposta += "info:\n";
      
      #ifdef ESP32
        resposta += "MCU: ESP32\n";
      #else
        resposta += "MCU: ESP8266\n";
      #endif

      resposta += "upload date: " + String( __DATE__ ) + "\n"  ;
      resposta += "upload time: " + String( __TIME__ ) + "\n"  ;
      resposta += "firmware: "    + String( __FIRMWARE__ ) + "\n";
      resposta += "ESPNOW_device:\n";
      resposta += "\tname: " + String( ESPNOW_DEVICE_NAME ) + "\n";
      resposta += ( ESPNOW_DEVICE_SERVER ? "\tserver\n" : "\tclient\n" );
      resposta += "\tconnections: " + String( ESPNOW_device.connections_counter ) + "\n";
      resposta += "\tconnections max: " + String( ESPNOW_device.connections_counter_max ) + "\n";

      Serial.println("Running sketch:");
      Serial.println();
      Serial.println("\n\n");
    }

    return resposta;
}