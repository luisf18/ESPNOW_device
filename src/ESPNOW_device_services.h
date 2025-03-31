#ifndef ESPNOW_DEVICE_SERVICES_H
#define ESPNOW_DEVICE_SERVICES_H

// ================================================================================
// Serviços transportados na comunicação
// ================================================================================
namespace ESPNOW_SERVICE {

  enum devices{
    
    // Robos genéricos
    ROBOT_DIF_2W = 1, // robo diferencial de 2 rodas
    ROBOT_DIF_4W,     // robo diferencial de 2 rodas

    // Robos genéricos
    ROBOT_OMINI_3W,     // robo ominidirecional de 3 rodas
    ROBOT_OMINI_4W,     // robo ominidirecional de 4 rodas
    
    // Drones genericos
    DRONE_4H = 20,    // robo diferencial de 2 rodas
    DRONE_6H,         // robo diferencial de 2 rodas
    DRONE_8H,         // robo diferencial de 2 rodas

    // Competition Robots
    ROBOT_SUMO_NANO = 100,
    ROBOT_SUMO_MICRO,
    ROBOT_SUMO_MINI,
    ROBOT_SUMO_MEGA,
    ROBOT_COMBAT_FAIRY,
    ROBOT_COMBAT_ANT,
    ROBOT_COMBAT_BEETLE,
    ROBOT_COMBAT_MIDDLE
  };

  enum services{
    // Legado versão anterior
    RADIO_LEGACY = 18,
    BATTERY_LEGACY, //NOTIFY,
    SHELL,
    // V2.0
    RADIO = 25
  };
 
  // ---------------------------------------------------------------------------------------
  // Battery
  // ---------------------------------------------------------------------------------------
  typedef struct __attribute__((packed)){
    enum BatteryID : uint8_t {
      NONE = 0,    // Sem bateria
      LiPo,        // LiPo
      LiPo_current // LiPo com monitoramento de corrente
    };
    BatteryID id;      // Define um valor padrão
    float     voltage; // Tensão da bateria (V)
    float     current; // Corrente da bateria (A)
    uint8_t   temp;    // temperatura (°C)
    uint8_t   cells;   // Número de células
    uint8_t   level;   // Nível da bateria (%)
  } battery_t;

  // ---------------------------------------------------------------------------------------
  // MCU
  // ---------------------------------------------------------------------------------------
  typedef struct __attribute__((packed)){
    enum MCU_ID : uint8_t {
      MCU_unknone = 0,
      MCU_ESP8266,
      MCU_ESP32,
      MCU_ESP32S2,
      MCU_ESP32S3,
      MCU_ESP32C3
    };
    MCU_ID   id;
    uint16_t voltage_mV;  // Tensão do mcu (mV)
    int16_t  current_mA;  // Corrente consumida na parte logica (mA)
    uint8_t  temp;     // Temperature (°C)
  } mcu_t;

  // ---------------------------------------------------------------------------------------
  // motor
  // ---------------------------------------------------------------------------------------
  typedef struct __attribute__((packed)){
    union{
      uint8_t mode;
      uint8_t temp;
    };
    int16_t RPM;
  }motor_t;

  typedef struct __attribute__((packed)){
    union{
      uint8_t mode;
      uint8_t temp;
    };
    int16_t current;
    int16_t RPM;
  } motor_current_t;

  // ---------------------------------------------------------------------------------------
  // Sensores
  // ---------------------------------------------------------------------------------------
  typedef struct __attribute__((packed)){
    uint8_t mode;
    int8_t temp;       // temperatura (°C)
    float  ax, ay, az; // aceleração (m/s²)
    float  gx, gy, gz; // rotação (graus/s)
    float  mx, my, mz; // magnetometro (mT)
    int16_t altitude;  // Altitude (m)
  } imu_t;

  typedef struct __attribute__((packed)){
    uint8_t  type; // 0 = desativado, 1 = temperatura/umidade, 2 = pressão, etc.
    int8_t   temperature; // graus
    uint8_t  humidity;    // 0 a 100%
    uint16_t pressure;
  } env_t;

  // coord GPS
  typedef struct __attribute__((packed)){
    uint8_t sats;
    int16_t lat, lon, alt; // Latitude, Longitude (10^-2)graus e Altitude (m)
  } gps_coordinates_t;

  // coord CAMERA - Dados do Sistema de Câmeras
  typedef struct __attribute__((packed)){
    uint8_t target_id;
    int16_t x, y, z; // Latitude, Longitude (10^-2)graus e Altitude (m)
  } camera_coordinates_t;

  typedef struct __attribute__((packed)){
    uint8_t mode; // 0 = desativado, 1 = GPS, 2 = Sistema de Câmeras
    uint16_t speed, heading; // speed -> (cm/s) e heading -> (10^-2)graus
    union{
      gps_coordinates_t gps;
      camera_coordinates_t cam;
    };
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
  }location_t;

  // ---------------------------------------------------------------------------------------
  // Radio
  // ---------------------------------------------------------------------------------------
  typedef struct __attribute__((packed)){
    // Memory map (bytes):
    //[ 6 MCU ][ 12 Bateria ][ 43 CHANNELS ][ 40 IMU ][ 19 Location ][ 5 env ][ 80 msg ]
    // 205 Bytes
    uint16_t device_type;
    mcu_t mcu;          // [ 6 bytes ] 
    battery_t battery;  // [ 12 bytes ] Estrutura para informações da bateria
    // [43 bytes] Canais de controle
    // - (0 = desativado)
    // - ( de 22 a 35 -> modo motor )
    // - ( de 36 a 43 -> modo motor_extended )
    uint8_t channels_mode;
    union {
      int16_t  ch[21]; // channesl (PWM, PPM, etc.)
      motor_t  motor[14];
      motor_current_t motor_current[8];
    };
    imu_t imu;           // [ 40 bytes ] Estrutura IMU (Inertial Measurement Unit)
    location_t location; // [ 19 bytes ] Estrutura para Localização (GPS ou Sistema Externo de Câmeras)
    env_t env;           // [ 5 bytes ] Sensores ambientais
    char msg[80];        // [ 80 bytes ] Mensagem de status ou debug
  }radio_t;

}

#endif