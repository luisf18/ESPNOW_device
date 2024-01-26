// ====================================================================== //
//  DEVICE SERVICE                                                        //
// ====================================================================== //

#define ESPNOW_DEVICE_SERVICE__GENERIC         0xF000
#define ESPNOW_DEVICE_SERVICE__RADIO_CHANNELS  0xF100
#define ESPNOW_DEVICE_SERVICE__BATTERY_MONITOR 0xF200
#define ESPNOW_DEVICE_SERVICE__UART            0xF300
#define ESPNOW_DEVICE_SERVICE__KEYBOARD        0xF400
#define ESPNOW_DEVICE_SERVICE__LED_STRIP       0xF500

// ====================================================================== //
//  DEVICE UTILITY                                                        //
// ====================================================================== //

// Radio
#define ESPNOW_DEVICE_UTILITY__RADIO_GENERIC   0xA000
#define ESPNOW_DEVICE_UTILITY__RADIO_PISTOL    0xA100
#define ESPNOW_DEVICE_UTILITY__RADIO_AIRPLANE  0xA200
#define ESPNOW_DEVICE_UTILITY__RADIO_NUNCHUCK  0xA300
#define ESPNOW_DEVICE_UTILITY__RADIO_DRONE     0xA400

// Robot
#define ESPNOW_DEVICE_UTILITY__ROBOT_GENERIC        0xB000
#define ESPNOW_DEVICE_UTILITY__ROBOT_SUMO           0xB100
#define ESPNOW_DEVICE_UTILITY__ROBOT_COMBAT         0xB200
#define ESPNOW_DEVICE_UTILITY__ROBOT_LINE_FOLLOWER  0xB300
#define ESPNOW_DEVICE_UTILITY__ROBOT_SOCCER         0xB400

// 1 - Server or Client
    // Radio
    // 2 - tipo de radio
    // 3 - quantidade de canais
    
    // 2 - tipo de robô (movimentação DDR)
    // 3 - categoria do robô
    // 4 - elemento de combate

    //enum{
    //  RADIO = 0x0000,
    //  RADIO_AIRPLANE,
    //  RADIO_DRONE,
    //  RADIO_NUNCHUCK,
    //  ROBOT = 0x1000,
    //  ROBOT_DDR_SUMO_MINI,
    //  ROBOT_DDR_SUMO_MEGA,
    //  ROBOT_DDR_COMBAT_FAIRY,
    //  ROBOT_DDR_COMBAT_FAIRY,
    //}

    /*/
    enum{

      MASK_DEVICE         = 0x1000,
      MASK_RADIO_TYPE     = 0x0100,
      MASK_RADIO_CHANNEL  = 0x0011,
      MASK_ROBOT_TYPE     = 0x0100,
      MASK_ROBOT_CATEGORY = 0x0010,
      MASK_ROBOT_WEAPON   = 0x0001,

      RADIO     = 0x0000,

      PISTOL    = 0x0000,
      AIRPLANE  = 0x0100,
      DRONE     = 0x0200,
      NUNCHUCK  = 0x0300,

      CHANNEL_1 = 1,
      CHANNEL_2,
      CHANNEL_3,
      CHANNEL_4,
      CHANNEL_5,
      CHANNEL_6,
      CHANNEL_7,
      CHANNEL_8,
      CHANNEL_9,
      CHANNEL_10,

      ROBOT     = 0x1000,
      
      // Tipo de movimento
      DDR       = 0x0000,
      
      // Categoria
      SUMO            = 0x0000,
      COMBAT          = 0x0010,
      FUTEBOL         = 0x0010,
      LINE_FOLLOWER   = 0x0010,

      // Weapons - combat
      NO_WEAPON          = 0x0000,
      VERTICAL_SPINNER   = 0x0001,
      HORIZONTAL_SPINNER = 0x0002,
      FLIPPER            = 0x0003,
      FORKS              = 0x0004,
      RAMP               = 0x0005,
      FLAGS              = 0x0006, // sumo
      ARMS               = 0x0007  // sumo
      
    }
    /*/

    //typedef struct device_t{
    //  char     name[15]  = "TX";
    //  bool     is_server  = false;
    //  uint32_t color_rgb = 0xF00000;
    //  uint32_t service   = 0; //ROBOT | DDR | SUMO | RAMP; // tipo de serviço que o dispositivo de origem esta prestando
    //}device_t;