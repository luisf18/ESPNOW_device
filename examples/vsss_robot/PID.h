// ================================================================================
// CLASSE CONTROLADOR PID
// ================================================================================

class control_pid{
  public:
  float kp = 1;
  float ki = 0;
  float kd = 0;
  uint32_t last_ms = 0;
  float last_erro = 0;
  float P = 0;
  float I = 0;
  float D = 0;
  float Imax = 1000;
  float loop(float erro){
    uint32_t ms = millis();
    uint32_t dt = ms - last_ms;
    if( dt == 0 ) dt = 1;
    P = erro;
    D = 1000.0*( erro - last_erro )/(float)dt;
    //I = constrain( I+ki*0.001*erro*dt, -I_MAX, I_MAX );
    I = constrain( I+0.001*erro*dt, -Imax, Imax );
    last_erro = erro;
    last_ms = ms;
    //return P*kp + I*ki + kd*D;
    return kp*P + I*ki + kd*D;
  }
  void init(){
    I = 0;
  }
};

// ================================================================================
// CONTROLADOR PID de velocidade angular com realimentação do giroscópio
// ================================================================================

// Strings para debug
String pid_log = "";
String pid_log_plot = "";

// controlador PID de velocidade angular
control_pid PID_w;

// set points
float pid_speed   = 0; // linear speed
float pid_speed_w = 0; // angular speed

int pid_w_loop( int lin, float w ){

  // -------------------------------------------------------------------------------------------------------------------------------------
  // executa controlador
  // -------------------------------------------------------------------------------------------------------------------------------------
  float w_real = (absf(IMU.g.gyro.z) < 0.01 ? 0.0 : IMU.g.gyro.z);
  int dif = PID_w.loop( w - w_real );
  int vl = 0, vr = 0;

  if( abs(lin) < 200 ){
    vl = lin + dif*0.5;
    vr = lin - dif*0.5;
  }else if( lin > 0 ){
    if( dif < 0 ){
      vl = lin+dif;
      vr = lin;
    }else{
      vl = lin;
      vr = lin - dif;
    }
  }else{
    if( dif > 0 ){
      vl = lin+dif;
      vr = lin;
    }else{
      vl = lin;
      vr = lin - dif;
    }
  }

  vl = constrain( vl, -1023, 1023 );
  vr = constrain( vr, -1023, 1023 );

  // -------------------------------------------------------------------------------------------------------------------------------------
  // Se estiver as rodas n estiverem viradas pro chão não mexe
  // -------------------------------------------------------------------------------------------------------------------------------------
  if( absf(IMU.ac_z_filter) > 4.0 ) motor.move( vl, vr );
  else motor.stop();

  // -------------------------------------------------------------------------------------------------------------------------------------
  // LOGGING
  // -------------------------------------------------------------------------------------------------------------------------------------
  pid_log =  "[pid: [ speed: " + String(vl) + "L * " + String(vr) + "R ] [ angle: " + String(IMU.gyro_z_angle*(180.0/PI)) + " deg ] [ P: " + String(PID_w.P) + " (kp:" + String(PID_w.kp) + " ) D: " + String(PID_w.D) + " (kd:" + String(PID_w.kd) + " ) I: " + String(PID_w.I) + " (ki:" + String(PID_w.ki) + " ) ] ]\n";
  
  float pc_E = ( w == 0.0 ? 0.0 : 100.0*PID_w.P/(float)w );
  float SUM = absf(PID_w.P*PID_w.kp) + absf(PID_w.I*PID_w.ki) + absf(PID_w.D*PID_w.kd);
  float pc_P = ( SUM == 0.0 ? 0.0 : constrain( 100.0*PID_w.P*PID_w.kp/SUM, -200, 200 ));
  float pc_I = ( SUM == 0.0 ? 0.0 : constrain( 100.0*PID_w.I*PID_w.ki/SUM, -200, 200 ));
  float pc_D = ( SUM == 0.0 ? 0.0 : constrain( 100.0*PID_w.D*PID_w.kd/SUM, -200, 200 ));
  float pc_O = 100.0*(float)dif/2048.0;

  pid_log_plot =
    //String(IMU.gyro_z_angle*(180.0/PI)) + " " +
    String(IMU.g.gyro.z) + " " +
    String(pc_E) + " " +
    String(pc_P) + " " +
    String(pc_I) + " " +
    String(pc_D) + " " +
    String(pc_O) + " " +
    String(PID_w.P) + " " +
    String(PID_w.I) + " " +
    String(PID_w.D) + " " +
    String(PID_w.kp) + " " +
    String(PID_w.ki) + " " +
    String(PID_w.kd) + " " +
    String(vl) + " " +
    String(vr) + "\n";

  return dif;
}

// ================================================================================
// TESTE e MEDIÇÃO DO CONTROLADOR PID
// ================================================================================

String pid_test( int lin, float w, uint16_t n, uint16_t dt ){
  String result = "";
  String line = "n t lin w gyro out\n";
  result += line;
  uint32_t timeout = 0;
  int i=0;
  while(i<n){
    IMU.update();
    int out = pid_w_loop( lin, w );
    if( millis() >= timeout ){
      i++;
      timeout = millis() + dt;
      line = String(i) + " " + String(millis()) + " " + String( lin ) + " " + String( w ) + " " + String( IMU.g.gyro.z ) + " " + String( out ) + "\n";
      result += line;
      Serial.print(line);
    }
  }
  motor.stop();
  
  espnow_device_print( result.c_str() );
  
  return result;
}

