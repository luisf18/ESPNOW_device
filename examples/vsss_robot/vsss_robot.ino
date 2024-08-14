

const char *__FIRMWARE__ = __FILE__;
float absf( float v ){ return ( v < 0 ? -v : v ); }

#include "DRV8833.h"
#include "IMU.h"
#include "pinmap.h"
#include "GPIOCORE_MINI.h"

//==========================================================================
// Hardware init
//==========================================================================

/*  Motores  */
DRV8833   motor  = DRV8833(
  PIN__MOTOR_IN1,
  PIN__MOTOR_IN2,
  PIN__MOTOR_IN3,
  PIN__MOTOR_IN4
);

/*  LED  */
//GPIO_CORE LED = GPIO_CORE( PIN__LED );

/*  Bateria  */
GPIO_INPUT BAT( PIN__BATTERY ); // Battery monitor

/*  Botões  */
GPIO_INPUT BTN_A( PIN__BTN_A ); // A - Boot button
GPIO_INPUT BTN_B( PIN__BTN_B ); // B

// ================================================================================
// CONTROLE DE EXECUÇÃO
// ================================================================================

// precisa ser aqui!
#include "ESPNOW_device_local.h"
#include "PID.h"

bool running = false;

void start(){
  motor.bip( 3, 50, 2000 );
  motor.stop();
  PID_w.init();
  running = true;
}

void stop(){
  motor.stop();
  motor.bip( 2, 200, 2000 );
  running = false;
}

void run() {
  if( running ){
    pid_w_loop( pid_speed, pid_speed_w );
  }
}

//==========================================================================
// Main
//==========================================================================
// precisam ser aqui!
#include "Terminal.h"

void setup() {

  // Begin Serial
  Serial.begin(115200);
  Serial.setTimeout(80);

  // Begin Motors
  motor.begin();
  motor.sound_vol(12);

  // Alerta de inicialização
  motor.bip( 2, 100, 2000, 0 ); // Motor 0
  motor.bip( 2, 100, 2000, 1 ); // Motor 1
  motor.bip( 1, 50, 5500 );
  motor.bip( 1, 50, 3500 );
  motor.bip( 1, 50, 1500 );

  // IMU start
  IMU.begin();

  // calibração da IMU, deixe o robô parado!
  motor.bip( 5, 50, 5500 );
  IMU.calibrate();
  motor.bip( 5, 50, 1500 );
  //LED.on(); // <<-- o led acende pra indicar que esta pronto

  // Buttons
  BTN_A.begin();
  BTN_B.begin();

  // Battery
  BAT.begin();
  BAT.set_voltage_range( 0.915, 19.79, 6.6 );
  //float voltage(){
  //  return (4.61e-3)*analogRead(34)+0.915;
  //}

  // ESPNOW_device
  Serial.println( "======== ESPNOW Device Server ========" );
  //ESPNOW_device.set_led( 2 );
  ESPNOW_device.set_handle_function( espnow_handle );
  ESPNOW_device.waiting_ms_disconnect = 500;
  ESPNOW_device.delay_ms_notify       = 200;
  ESPNOW_device.begin_server( "ROBOT", "1324" );

  // Informações gerais do código
  Serial.println(Terminal("info"));

  // inicia o controlador de velocidade angular
  PID_w.kp = 100;
  PID_w.ki = 1500;
  PID_w.kd = 0.8;
  PID_w.Imax = 1;

}

void loop() {
  
  // IMU
  IMU.update();
  ESPNOW_device.update();

  run();

  while(Serial.available() > 0){
    String CMD = Serial.readStringUntil(';');
    //String CMD = SerialBT.readString();
    Serial.println(CMD);
    //ESPNOWSerial.println( CMD );
    String resposta = Terminal(CMD.c_str());
    Serial.println(resposta);
    //ESPNOWSerial.println(resposta);
  }

  if( ESPNOW_device_cmd.length() > 0 ){
    String CMD = ESPNOW_device_cmd;
    while( CMD.length() > 0 ){
      int i = CMD.indexOf(';');
      if( i < 0 ){
        Serial.println(CMD);
        String resposta = Terminal(CMD.c_str());
        Serial.println(resposta);
        break;
      }else{
        String CMD_part = CMD.substring( 0, i );
        CMD.remove( 0, i+2 );
        String resposta = Terminal(CMD_part.c_str());
        Serial.println(resposta);
      }
    }
  }

}
