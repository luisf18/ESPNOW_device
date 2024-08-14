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


// ==============================================================
// Terminal: interpretador de comandos embarcado
// ==============================================================

String Terminal( const char *const cmd ){
    
    // obtem os valores
    char key[20]; float x,y,z;
    int i = sscanf(cmd,"%s %f %f %f ",key,&x,&y,&z);
    if(i==0) return "> fail";

    String resposta = "> ";

    // --------------------------------------------------------------
    // MCU
    // Reset / MAC / digitalRead / digitalWrite
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
    }else if ( strcmp(key,"analogRead") == 0 ) {
        resposta += "analogRead: ";
        if( i > 1 ){
          x = constrain( x, 0, 40 ); // pin
          resposta += "pin " + String( (int)x ) + " -> " + String( analogRead((int)x) ) + "\n";
        }else{
          resposta += "fail\n";
        }
    }
    
    // --------------------------------------------------------------
    // Sensoriamento
    // Voltage / IMU
    // --------------------------------------------------------------
    else if (strcmp(key,"voltage") == 0 ) {
        resposta += "Voltage: " + String(BAT.voltage()) + " V\n";
    }else if (strcmp(key,"IMU") == 0 ) {
        //IMU.update();
        resposta += ( "ac(x,y,z)[m/s²]: "    + String(IMU.a.acceleration.x,2) + " " + String(IMU.a.acceleration.y,2) + " " + String(IMU.a.acceleration.z,2) + "\n" );
        resposta += ( "gyro(x,y,z)[rad/s]: " + String(IMU.g.gyro.x        ,2) + " " + String(IMU.g.gyro.y        ,2) + " " + String(IMU.g.gyro.z        ,2) + "\n" );
        resposta += ( "temp[°C]: "           + String(IMU.temp.temperature,2) + "\n" );
    }
    
    //else if (strcmp(key,"IMU") == 0 ) {
    //    //IMU.update();
    //    resposta += ( "ac(x,y,z)[m/s²]: "    + String(IMU.a.acceleration.x,2) + " " + String(IMU.a.acceleration.y,2) + " " + String(IMU.a.acceleration.z,2) + "\n" );
    //    resposta += ( "gyro(x,y,z)[rad/s]: " + String(IMU.g.gyro.x        ,2) + " " + String(IMU.g.gyro.y        ,2) + " " + String(IMU.g.gyro.z        ,2) + "\n" );
    //    resposta += ( "temp[°C]: "           + String(IMU.temp.temperature,2) + "\n" );
    //}

    // --------------------------------------------------------------
    // Help
    // --------------------------------------------------------------
    else if (strcmp(key,"help") == 0 ) {
        resposta += "help list:\n";
        resposta += "speed_tg:\t" + String(pid_speed  ) + "\n";
        resposta += "speed_w:\t"  + String(pid_speed_w) + "\n";
        resposta += "Kp:\t"       + String(PID_w.kp) + "\n";
        resposta += "Ki:\t"       + String(PID_w.ki) + "\n";
        resposta += "Kd:\t"       + String(PID_w.kd) + "\n";
        resposta += "P:\t"        + String(PID_w.P) + "\n";
        resposta += "I:\t"        + String(PID_w.I) + "\n";
        resposta += "D:\t"        + String(PID_w.D) + "\n";
        resposta += "Imax:\t"     + String(PID_w.Imax) + "\n";
        resposta += "outros comandos:\n";
        resposta += "reset\n";
        resposta += "start\n";
        resposta += "stop\n";
        resposta += "speed\n";
        resposta += "w\n";
        resposta += "help\n";
    }
    
    // --------------------------------------------------------------
    // firmware informations
    // --------------------------------------------------------------
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
      Serial.println("\n\n");
    }



    // --------------------------------------------------------------
    // Motor
    // --------------------------------------------------------------
    else if (strcmp(key,"move") == 0 ) {
             if(i == 2) motor.move( constrain(x,-1023,1023), constrain(x,-1023,1023) );
        else if(i >= 3) motor.move( constrain(x,-1023,1023), constrain(y,-1023,1023) );
        resposta += "move: " + String(motor.read(0)) + " " + String(motor.read(1)) + "\n";
        if( i==4 ){
          z = abs(z);
          z = constrain(z,0,10000);
          resposta += String(z) + " ms ";
          delay(z);
          motor.stop();
        }
        resposta += "\n";
    }else if (strcmp(key,"bip") == 0 ) {
        x = ( i > 1 ? constrain(x,1,100 ) : 2    ); // n
        y = ( i > 2 ? constrain(y,1,2000) : 100  ); // delay
        z = ( i > 3 ? constrain(z,1,1e9 ) : 2000 ); // frequência
        motor.bip( x, y, z );
        resposta += "bip\n";
    }else if (strcmp(key,"motor.stop") == 0 ) {
        resposta += "stop\n";
        motor.stop();
    }else if (strcmp(key,"motor.off") == 0 ) {
        resposta += "off\n";
        motor.off();
    }

    /*/
    else if (strcmp(key,"start") == 0 ) {
        resposta += "start";
        start();
    } else if (strcmp(key,"stop") == 0 ) {
        resposta += "stop";
        stop();
    }
    /*/
    
    // --------------------------------------------------------------
    // Controladores de sistemas
    // --------------------------------------------------------------

    //-- controle de estado --//
    else if (strcmp(key,"start") == 0 ) {
        resposta += "start";
        start();
    } else if (strcmp(key,"stop") == 0 ) {
        resposta += "stop";
        stop();
    }

    //-- constante do controlador --//
    else if (strcmp(key,"kp") == 0) {
        if(i==2) PID_w.kp = x;
        resposta += "Kp: " + String(PID_w.kp) + "\n";
    } else if (strcmp(key,"ki") == 0 ) {
        if(i==2) PID_w.ki = x;
        resposta += "Ki: " + String(PID_w.ki) + "\n";
    } else if (strcmp(key,"kd") == 0 ) {
        if(i==2) PID_w.kd = x;
        resposta += "Kd: " + String(PID_w.kd) + "\n";
    }

    //-- variaveis de estado do controlador --//
    else if (strcmp(key,"P") == 0) {
        if(i==2) PID_w.P = x;
        resposta += "P: " + String(PID_w.P) + "\n";
    } else if (strcmp(key,"I") == 0 ) {
        if(i==2) PID_w.I = x;
        resposta += "I: " + String(PID_w.I) + "\n";
    } else if (strcmp(key,"D") == 0 ) {
        if(i==2) PID_w.D = x;
        resposta += "D: " + String(PID_w.D) + "\n";
    } else if (strcmp(key,"speed") == 0 ) {
        if(i==2) pid_speed = constrain(x,-1023,1023);
        resposta += "speed: " + String(pid_speed) + "\n";
    } else if (strcmp(key,"w") == 0 ) {
        if(i==2) pid_speed_w = x;
        resposta += "speed_w: " + String(pid_speed_w) + "\n";
    }


    // --------------------------------------------------------------
    // Armazenamento e configurações
    // --------------------------------------------------------------
    // ...

    return resposta;
}