#ifndef ESPNOW_DEVICE_SERVICES_H
#define ESPNOW_DEVICE_SERVICES_H

// os serviços padrões que podem ser transmitidos por espnow device

enum ESPNOW_device_service_t{
   ESPNOW_DEVICE__SERVICE_RADIO_BASIC = 18,
   ESPNOW_DEVICE_SERVICE_TELEMETRY, //ESPNOW_DEVICE_SERVICE_NOTIFY,
   ESPNOW_DEVICE_SERVICE_SHELL,
   ESPNOW_DEVICE__SERVICE_RADIO = 25
};

// ================================================================================
// ESPNOW_DEVICE_service class
// ================================================================================

class ESPNOW_DEVICE_service{
  public:
  //static const uint16_t ID = 0; // Não ocupa espaço por instância
  //static const uint8_t LEN = 0; // Não ocupa espaço por instância
  uint16_t id = 0;
  uint8_t  len = 0;

  virtual bool ok(){
    return false; //( (ID == id) && (len == LEN) );
  }
  virtual bool load( const uint8_t *data ){
    ESPNOW_DEVICE_service *temp = (ESPNOW_DEVICE_service*) data;
    if( temp->id == id && temp->len == len ){
      //*this = *temp;
      memcpy( ((uint8_t*)this), data, len );
      return true;
    }
    return false;
  }
  virtual void set( uint8_t *data ){
    memcpy( data, (uint8_t *) this, len );
  }

  ESPNOW_DEVICE_service(){
    //id = ID;
    //len = LEN;
  }

};

// ================================================================================
// Radio - basico
// ================================================================================

class ESPNOW_DEVICE_service_radio_basic : public ESPNOW_DEVICE_service{
  public:
  //static const uint16_t ID = ESPNOW_DEVICE__SERVICE_RADIO_BASIC;
  //static const uint8_t  LEN = 40;

  ESPNOW_DEVICE_service_radio_basic(){
    id  = ESPNOW_DEVICE__SERVICE_RADIO_BASIC;
    len = 40;
  }

  bool load( const uint8_t *data ) override {
    ESPNOW_DEVICE_service *temp = (ESPNOW_DEVICE_service_radio_basic*) data;
    //Serial.printf( "| %d  ", temp->ID );
    Serial.printf( "| %d  ", temp->id );
    //Serial.printf( "| %d  ", temp->LEN );
    Serial.printf( "| %d  ", temp->len );
    if( temp->id == ESPNOW_DEVICE__SERVICE_RADIO_BASIC ){
      memcpy( ((uint8_t*)this), data, len );
      return true;
    }
    return false;
  }

  uint8_t channelCount(){
    return (len/2);
  }

  void setChannel( uint8_t n ){
    len = constrain( n*2, 0, 20 );
  }

  void setBattery( ){
    len = 6;
  }

  typedef struct{
    uint8_t cells   = 2;
    float   voltage = 0;
    uint8_t level   = 0;
  } BATTERY;

  union{
    int16_t  ch[20];
    BATTERY battery;
  };
};

// ================================================================================
// Radio remote service
// ================================================================================

class ESPNOW_DEVICE_service_radio : public ESPNOW_DEVICE_service{
  
  public:
  static const uint16_t ID = ESPNOW_DEVICE__SERVICE_RADIO;
  static const uint8_t LEN = 3+205;

  ESPNOW_DEVICE_service_radio(){
    id = ID;
    len = LEN;
  }
  
  // Memory map (bytes):
  //[ 6 MCU ][ 12 Bateria ][ 43 CHANNELS ][ 40 IMU ][ 19 Location ][ 5 env ][ 80 msg ]
  // 205 Bytes

  uint16_t device_type = 0;

  // Estrutura para informações da bateria
  // 6 bytes
  struct MCU {
    enum MCU_ID : uint8_t {
      MCU_unknone = 0,
      MCU_ESP8266,
      MCU_ESP32,
      MCU_ESP32S2,
      MCU_ESP32S3,
      MCU_ESP32C3
    };
    MCU_ID   id = MCU_ID::MCU_ESP32;
    uint16_t voltage_mV = 0;  // Tensão do mcu (mV)
    int16_t  current_mA = 0;  // Corrente consumida na parte logica (mA)
    uint8_t  temp    = 0;     // Temperature (°C)
  } mcu;
  
  // Estrutura para informações da bateria
  // 12 bytes
  struct BATTERY {
    enum BatteryID : uint8_t {
      NONE = 0,    // Sem bateria
      LiPo,        // LiPo
      LiPo_current // LiPo com monitoramento de corrente
    };
    BatteryID id = BatteryID::LiPo; // Define um valor padrão
    float     voltage = 0;  // Tensão da bateria (V)
    float     current = 0;  // Corrente da bateria (A)
    uint8_t   temp    = 0;  // temperatura (°C)
    uint8_t   cells   = 0;  // Número de células
    uint8_t   level   = 0;  // Nível da bateria (%)
  } battery;

  // Canais de controle
  // 43 bytes
  // Quantidade de canais ativos
  // - (0 = desativado)
  // - ( de 22 a 35 -> modo motor )
  // - ( de 36 a 43 -> modo motor_extended )
  uint8_t channels_mode = 0;

  typedef struct{
    union{
      uint8_t mode = 0;
      uint8_t temp;
    };
    int16_t RPM = 0;
  }MOTOR;

  typedef struct{
    union{
      uint8_t mode = 0;
      uint8_t temp;
    };
    int16_t current;
    int16_t RPM = 0;
  } MOTOR_extended;

  union {
    // channesl (PWM, PPM, etc.)
    int16_t  ch[21];
    // motor
    MOTOR motor[14];
    MOTOR_extended motor_extended[8];
  };

  // Estrutura IMU (Inertial Measurement Unit)
  // 40 bytes
  struct IMU {
    uint8_t mode = 0;
    //uint8_t precision = 0; // Precisão dos dados (0 = desativado)
    int8_t temp;       // temperatura (°C)
    float  ax, ay, az; // aceleração (m/s²)
    float  gx, gy, gz; // rotação (graus/s)
    float  mx, my, mz; // magnetometro (mT)
    int16_t altitude;  // Altitude (m)
  } imu;

  // Estrutura para Localização (GPS ou Sistema Externo de Câmeras)
  // 19 bytes
  
  // coord GPS
  typedef struct{
    uint8_t sats;
    int16_t lat, lon, alt; // Latitude, Longitude (10^-2)graus e Altitude (m)
  } GPS_coordinates;

  // coord CAMERA - Dados do Sistema de Câmeras
  typedef struct{
    uint8_t target_id;
    int16_t x, y, z; // Latitude, Longitude (10^-2)graus e Altitude (m)
  } CAMERA_coordinates;

  struct LocationData {
    uint8_t mode = 0; // 0 = desativado, 1 = GPS, 2 = Sistema de Câmeras
    uint16_t speed, heading; // speed -> (cm/s) e heading -> (10^-2)graus
    union{
      GPS_coordinates gps;
      CAMERA_coordinates cam;
    };
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t min = 0;
    uint8_t sec = 0;
  }location;

  // Sensores ambientais
  // 5 bytes
  struct ENVIRONMENT {
    uint8_t  type = 0; // 0 = desativado, 1 = temperatura/umidade, 2 = pressão, etc.
    int8_t   temperature; // graus
    uint8_t  humidity;    // 0 a 100%
    uint16_t pressure;
  } env;

  char msg[80];  // Mensagem de status ou debug

};

#endif