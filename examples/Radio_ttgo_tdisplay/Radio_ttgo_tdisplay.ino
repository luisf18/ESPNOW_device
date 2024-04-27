#include "TTGO_T1.h"
#include "ESPNOW_device.h"


#define COLOR_lilas            RGB_24(  60,   0, 250 )
#define COLOR_azul_esverdeado  RGB_24(   0, 255,  60 )
#define COLOR_azul_claro       RGB_24(   0, 160, 200 )
#define COLOR_azul_escuro      RGB_24(   0,   0, 255 )
#define COLOR_laranja          RGB_24( 255, 100,   0 )
#define COLOR_amarelo          RGB_24( 255, 160,   0 )

const char *__FIRMWARE__ = __FILE__;

// =================================================================
// PINOUT
// =================================================================
#define CH1     -1   // X
#define CH2     -1   // Y
#define CH3     27   // Z (NC)
//#define CH4     6   // Z (NC)
//#define CH5     6   // Z (NC)
#define CH4     -1  // SWD
#define CH5     -1   // SWA (Mode)
#define LED     -1   // LED
#define NEW_LED CH3 // 
#define BUZZER  -1
#define BATTERY -1

// =================================================================
// CALIBRAÇÃO
// =================================================================
int cal_x0, cal_x0_up, cal_x0_down;
int cal_y0, cal_y0_up, cal_y0_down;

// =================================================================
// PIXEL
// =================================================================
#include "PIXEL.h"
Pixel PX(NEW_LED,2);
uint32_t px_color = PX_RED;

#define COLOR_ON_CALIBRATION    PX_RED
#define COLOR_ON                PX_BLUE // COLOR_azul_claro
#define COLOR_RADIO_LOW_BATTERY PX_RED
#define COLOR_ROBOT_LOW_BATTERY COLOR_amarelo

uint32_t COLOR_CONNECT = PX_GREEN;

void px_set_color_raw( uint32_t _px_color ){
  PX.set(_px_color,0);
  PX.write();
}

void px_set_color( uint32_t _px_color ){
  px_color = _px_color;
  PX.set(px_color,0);
  PX.write();
}

// =================================================================
// Robots list
// =================================================================

class robot_connection{
  public:
    const char * name     = "ROBOT";
    const char * password = "1234";
    uint32_t     color    = 0x0000FF;
    // Construtor que recebe valores de variáveis
    robot_connection(const char *name, const char *password, uint32_t color) : name(name), password(password), color(color) {}
};

robot_connection ROBOT(  "ROBOT",  "1324", COLOR_lilas           );
robot_connection SERVER( "SERVER", "1234", COLOR_azul_esverdeado );

// =================================================================
// Battery voltage
// =================================================================
#include "esp_adc_cal.h"
uint32_t Timeout_battery_check     = 0;
uint32_t Timeout_low_battery_blink = 0;

// flags
bool     battery_low               = false;
bool     low_battery_blink_state   = 0;

//uint32_t voltage_mv(){
//  int ADC_Raw = analogRead(BATTERY);
//  esp_adc_cal_characteristics_t adc_chars;
//  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
//  return 2*esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars);
//}

float voltage(){
  return analogRead(34)*(6.6/4095.0); // voltage_mv()/1000.0;
}

void battery_update(){

  uint32_t time = millis();
  
  if( Timeout_battery_check <= time ){
    Timeout_battery_check = time + 1000;
    battery_low = ( voltage() <= 3.3 );
    Timeout_low_battery_blink = 0;
    low_battery_blink_state = 0;
    Serial.printf( "[ RADIO BATTERY: %f V ]\n", voltage() );
  }

  if( battery_low ){
    if( Timeout_low_battery_blink <= time ){
      Timeout_low_battery_blink = time + ( low_battery_blink_state ? 400 : 100 );
      px_set_color_raw( low_battery_blink_state ? px_color : COLOR_RADIO_LOW_BATTERY );
      low_battery_blink_state = !low_battery_blink_state;
    }
  }else{
    if( low_battery_blink_state ){
      low_battery_blink_state = 0;
      px_set_color( px_color );
    }
  }

}

// =================================================================
// BUZZER
// =================================================================
void bip(int N, int dt){
  dt = dt/2;
  for(int i=0;i<N;i++){
    digitalWrite(BUZZER,LOW ); delay(dt);
    digitalWrite(BUZZER,HIGH); delay(dt);
  }
}

// =================================================================
// ESPNOW DEVICE
// =================================================================

enum ESPNOW_device_service{
   ESPNOW_DEVICE_CHANNEL_SERVICE = 18,
   ESPNOW_DEVICE_NOTIFY_SERVICE
};

typedef struct{
  uint8_t cells   = 2;
  float   voltage = 0;
  uint8_t level   = 0;
} notify_t;


void display_onoff_connection( bool Flag_online ){
  T1.Lcd.fillRect(1,111,58,23, Flag_online ? GREEN : RED );
  T1.Lcd.setCursor(Flag_online?25:20,116);
  T1.Lcd.setTextColor(BLACK);
  T1.Lcd.println( "OK" );
}

void espnow_handle( espnow_device_event_t ev, int id ){
  
  if( ev == ESPNOW_EVT_NOTIFY ){

    T1.Lcd.setTextSize(1);
    T1.Lcd.setTextFont(2);

    display_onoff_connection( true );

    //bateria
    T1.Lcd.fillRect(61,111,58,23,BLACK);
    T1.Lcd.setTextColor(WHITE);
    T1.Lcd.setCursor(71,115);
    T1.Lcd.printf( "%.2fV", analogRead(34)*(6.6/4095.0) );

    Serial.printf( "[NOTIFY -> %s]\n", ( id >= 0 ? ESPNOW_device.connections[id].remote_name : "public" ) );
    
    ESPNOW_device.connections[0].frame_out.service = ESPNOW_DEVICE_CHANNEL_SERVICE;
    ESPNOW_device.connections[0].frame_out.len = sizeof(int)*8;
    int *ch = (int*) ESPNOW_device.connections[0].frame_out.body;


    int an0 = analogRead(32) - 1750;
    int an1 = analogRead(33) - 1700; // 1200 1700 2800
    if( abs(an0) < 60 ) an0 = 0;
    if( abs(an1) < 60 ) an1 = 0;

    if( an0 >= 0 ){ an0 = map( an0, 0, 3200-1700, 1500, 2000 ); }else{ an0 = map( an0, 0,  200-1700, 1500, 1000 ); }
    if( an1 >= 0 ){ an1 = map( an1, 0, 2750-1700, 1500, 2000 ); }else{ an1 = map( an1, 0, 1300-1700, 1500, 1000 ); }



    //int an1 = analogRead(CH1);
    //int an2 = analogRead(CH2);
    //ch[0] = 1500;
    //ch[1] = 1500;
    //if( an1 > cal_x0_up   ) ch[0] -= map( an1, cal_x0_up  , 2800, 0, 500 );
    //if( an1 < cal_x0_down ) ch[0] += map( an1, cal_x0_down, 1900, 0, 500 );
    //if( an2 > cal_y0_up   ) ch[1] -= map( an2, cal_y0_up  , 3000, 0, 500 );
    //if( an2 < cal_y0_down ) ch[1] += map( an2, cal_y0_down, 1850, 0, 500 );

    ch[0] = constrain(an0,1000,2000);
    ch[1] = constrain(an1,1000,2000);
    ch[2] = 1000*(digitalRead(0)==0) + 1000;
    ch[3] = 1500;
    ch[4] = 1500; //(digitalRead(CH5)?1000:2000);
    ch[5] = 1500; //(digitalRead(CH4)?1000:2000);
    ch[6] = 1000;
    ch[7] = 1000;

    Serial.printf(
      "CH [ %d\t%d\t%d\t%d\t%d\t%d\t%d\t%d ] ",
      ch[0],
      ch[1],
      ch[2],
      ch[3],
      ch[4],
      ch[5],
      ch[6],
      ch[7]
    );
  
  }else if( ev == ESPNOW_EVT_PUBLIC_NOTIFY ){

    Serial.printf( "[PUBLIC NOTIFY -> %s]\n", ( id >= 0 ? ESPNOW_device.connections[id].remote_name : "broadcast" ) );
  
  }else if( ev == ESPNOW_EVT_DISCONNECTED ){

    T1.Lcd.setTextSize(1);
    T1.Lcd.setTextFont(2);

    display_onoff_connection( false );

    px_set_color( COLOR_ON );
    Serial.printf( "[FALL -> %s]\n", ( id >= 0 ? ESPNOW_device.connections[id].remote_name : "broadcast" ) );
    bip( 2, 100 );
  
  }else if( ev == ESPNOW_EVT_CONNECTED ){

    COLOR_CONNECT = PX_GREEN;

    const char * name = ESPNOW_device.connections[id].remote_name;

    if( strncmp( name, ROBOT.name , ESPNOW_DEVICE_NAME_SIZE ) == 0 ) COLOR_CONNECT = ROBOT.color;
    if( strncmp( name, SERVER.name, ESPNOW_DEVICE_NAME_SIZE ) == 0 ) COLOR_CONNECT = SERVER.color;

    px_set_color( COLOR_CONNECT );
    Serial.printf( "[RISE -> %s]\n", ( id >= 0 ? ESPNOW_device.connections[id].remote_name : "broadcast" ) );
    bip( 1, 50 );
  
  }else if( ev == ESPNOW_EVT_RECIVE ){
  
    Serial.printf( "[RECIVE -> %s]\n", ( id >= 0 ? ESPNOW_device.connections[id].remote_name : "broadcast" ) );

    if( ESPNOW_device.connections[0].frame_in.service = ESPNOW_DEVICE_NOTIFY_SERVICE && ESPNOW_device.connections[0].frame_in.len >= sizeof( notify_t ) ){
      notify_t *notify_pack = (notify_t*) ESPNOW_device.connections[0].frame_in.body;
      Serial.printf( "[ voltage: %f ]\n", notify_pack->voltage );
      if( notify_pack->voltage < 3.35*notify_pack->cells ){
        px_set_color( COLOR_ROBOT_LOW_BATTERY );
        bip( 1, 50 );
      }else{
        px_set_color( COLOR_CONNECT );
      }
    }
  
  }
  
}

// =================================================================
// MAIN
// =================================================================

#include "Terminal.h"

void setup() {

  // put your setup code here, to run once:
  T1.begin();
  T1.Lcd.setRotation(3);
  // upload info ////////////////////////////
  Serial.println("\n\n\n\nUPLOAD Date:");
  Serial.println(__DATE__);
  Serial.println("UPLOAD Time:");
  Serial.println(__TIME__);
  Serial.println("Running sketch:");
  Serial.println(__FILE__);
  Serial.println("\n\n");

  // inicia o LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);

  // Input
  pinMode(CH1,INPUT);
  pinMode(CH2,INPUT);
  //pinMode(CH3,INPUT);
  pinMode(CH4,INPUT_PULLUP);
  pinMode(CH5,INPUT_PULLUP);
  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,HIGH); // off

  // Pixel
  PX.begin();

  // Serial
  Serial.begin(115200);
  Serial.setTimeout(20);

  T1.Lcd.setTextSize(1);
  T1.Lcd.setTextFont(2);
  display_onoff_connection( false );

  // calibração
  /*
  px_set_color( COLOR_ON_CALIBRATION );
  bip(3,150);
  
  float x0=0, y0=0;
  for(int i=0;i<5;i++){ x0+=(analogRead(CH1)/5.0); y0+=(analogRead(CH2)/5.0); delay(50); }
  cal_x0      = x0;
  cal_x0_up   = x0+50;
  cal_x0_down = x0-50;
  cal_y0      = y0;
  cal_y0_up   = y0+50;
  cal_y0_down = y0-50;
  Serial.printf("Calibração: [ X: %d - %d - %d ] [ Y: %d - %d - %d ]\n", cal_x0_down, cal_x0, cal_x0_up, cal_y0_down, cal_y0, cal_y0_up  );
  
  px_set_color( COLOR_ON );
  */

  // ESPNOW_device
  Serial.println( "======== ESPNOW Device Client ========" );
  ESPNOW_device.set_handle_function( espnow_handle );
  ESPNOW_device.connections_counter_max = 3;
  ESPNOW_device.single_server = true;
  ESPNOW_device.set_led( LED );
  ESPNOW_device.waiting_ms_disconnect = 500;
  ESPNOW_device.delay_ms_notify       = 50;

  ESPNOW_device.begin_client( "RADIO" );
  ESPNOW_device.connect( "ROBOT", "1324" );
  ESPNOW_device.connect( "SERVER", "1324" );
  ESPNOW_device.connect( "Miyagi", "1324" );

}

void loop() {
  ESPNOW_device.update();
  battery_update();

  while( Serial.available() > 0 ){
    String CMD = Serial.readStringUntil(';');
    CMD.trim();
    if( CMD.length() ){
      Serial.println("command: " + CMD);
      String resposta = terminal(CMD.c_str());
      Serial.println(resposta);
    }
  }

}

