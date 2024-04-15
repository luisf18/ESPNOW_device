#ifdef ESP32
  #include <esp_mac.h>
  #include <esp_now.h>
  #include <WiFi.h>
  #include <esp_wifi.h>
#elif defined(ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#endif

#define ESPNOW_DEVICE_CODE 18555
const uint8_t espnow_device_broadcast_mac[6]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#define ESPNOW_DEVICE_BODY_SIZE 210
#define ESPNOW_DEVICE_NAME_SIZE 16


//========================================================================================
// callback functions
//========================================================================================
void espnow_device_recive(const uint8_t * mac,const uint8_t *incomingData, int len);

//========================================================================================
// basic functions
//========================================================================================
String mac2str(const uint8_t *mac ){
  char char_str[18];
  snprintf(char_str, sizeof(char_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return char_str;
}

//========================================================================================
// Estruturas
//========================================================================================
typedef struct{
  // HEADER CODE [ 2 Bytes ] --------------------------------------------------------------
  uint16_t code        = ESPNOW_DEVICE_CODE; // codigo de identificação       [  2 Bytes ]
  // HEADER [ 22 Bytes ] ------------------------------------------------------------------
  char     name[ESPNOW_DEVICE_NAME_SIZE] = "ESPNOW_DEVICE";  // Nome do dispositivo origem    [ 15 Bytes ]
  // HEADER LOCK [ 19 Bytes ] -------------------------------------------------------------
  uint16_t random      = 0;                  // Dados de segurança da conexão [  2 Bytes ]
  char     name_rx[ESPNOW_DEVICE_NAME_SIZE] = "RX"; // Dispositivo destino    [ 15 Bytes ]
  // Body (carga util) [ 212 Bytes ] ------------------------------------------------------
  uint16_t service     = 0;                  // identificador do serviço      [  2 Bytes ]
  uint8_t  len         = 0;                  // tamanho da carga              [ *2 Bytes ]
  uint8_t  body[ESPNOW_DEVICE_BODY_SIZE]; //                               [ 0-200 Bytes ]
}espnow_device_frame_t;


//========================================================================================
// Credenciais
//========================================================================================
char     espnow_device_local_name[ESPNOW_DEVICE_NAME_SIZE] = "SERVER";
String   espnow_device_local_password = "banana";
uint32_t espnow_device_local_type     = 0;
bool     espnow_device_local_server   = true;

// connexões
//bool    always_public_notify = false;
//uint8_t connections_counter_max = 0;
//uint8_t connections_counter = 0;
//ESPNOW_device_connection connections[espnow_device_connection_max];


//========================================================================================
// Peer
//========================================================================================
bool peer( const uint8_t *mac, esp_now_peer_info_t *peerInfo ){
  // Register peer device
  memcpy(peerInfo->peer_addr, mac, 6);
  //peerInfo->channel = espnow_device_channel;
  peerInfo->encrypt = false;
  // Add peer
  if (esp_now_add_peer(peerInfo) != ESP_OK){ Serial.println("Failed to add peer"); return false; }
  return true;
}

//int find_name( const char *name ){
//  for(int i=0;i<espnow_device_connections_counter;i++){ if( strncmp(connections[i].name, name, 15) == 0 ) return i; }
//  return -1;
//}


//========================================================================================
// Connection
//========================================================================================
class ESPNOW_device_connection{

  #ifdef ESP32
  esp_now_peer_info_t peerInfo;
  #endif

  public:

  // remote data
  char    remote_name[ESPNOW_DEVICE_NAME_SIZE] = "ESPNOW_DEVICE";
  String  remote_password = "1234";
  uint8_t remote_mac[6]   = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  
  // connection info
  bool Change = false;
  bool Connected = false;
  bool searching_mac = false;

  // conexão
  espnow_device_frame_t frame_in, frame_out;

  // temporização
  uint32_t Timeout_disconnect    = 0;
  uint32_t waiting_ms_disconnect = 1000;
  uint32_t Timeout_notify        = 0;
  uint32_t delay_ms_notify       = 500;


  // ---------------------------------------------------------------------
  // Connection init
  // ---------------------------------------------------------------------

  // set mac
  void set_mac( const uint8_t *mac ){
    memcpy( remote_mac, mac, 6 );
    #ifdef ESP32
      peer( remote_mac, &peerInfo );
    #elif defined(ESP8266)
      peer( remote_mac );
    #endif
    searching_mac = false;

    if( memcmp( mac, espnow_device_broadcast_mac, 6 ) == 0 ){
      Serial.println( ">> connect broadcast notifier!" );
    }else{
      Serial.printf( ">> connect to remote %s %s!\n", (espnow_device_local_server?"client":"server"), remote_name );
    }
  }
  
  // conexão em um server
  void connect_server( const char *name, const char *_password ){

    // Remote info
    strncpy( remote_name, name, ESPNOW_DEVICE_NAME_SIZE );
    remote_password = _password;
    
    // frame header
    strncpy( frame_out.name, espnow_device_local_name, ESPNOW_DEVICE_NAME_SIZE );
    strncpy( frame_out.name_rx, remote_name, ESPNOW_DEVICE_NAME_SIZE );

    // connection flags
    Change = false;
    Connected = false;
    searching_mac = true;

    Serial.printf( ">> request connection to server %s!\n", name);
  }


  // um dispositivo client se connectou ao dispositivo local
  void connect_client( const char *name, const uint8_t *mac ){
    
    if( !espnow_device_local_server ) return;

    // Remote info
    strncpy( remote_name, name, ESPNOW_DEVICE_NAME_SIZE );
    
    // frame header
    strncpy( frame_out.name, espnow_device_local_name, ESPNOW_DEVICE_NAME_SIZE );
    strncpy( frame_out.name_rx, remote_name, ESPNOW_DEVICE_NAME_SIZE );
    
    // connection flags
    Change = true;
    Connected = true;

    // peer
    set_mac( mac );

  }

  //----------------------------------------------------------------------------------------
  // Send
  //----------------------------------------------------------------------------------------
  // armazena em frame_out.body os bytes de load depois envia frame_out
  void send( uint8_t *load, uint8_t load_len ){
    frame_out.len = constrain(load_len,0,ESPNOW_DEVICE_BODY_SIZE);
    memcpy( (uint8_t*)frame_out.body, load, frame_out.len );
    send();
  }

  // envia frame_out
  void send(){
    esp_now_send( remote_mac, (uint8_t*)&frame_out, sizeof(frame_out) - (ESPNOW_DEVICE_BODY_SIZE-frame_out.len) );
  }

  // ---------------------------------------------------------------------
  // loop
  // ---------------------------------------------------------------------

  void update( bool broadcast_notifier = false ){

    // broadcast notifier
    uint32_t time = millis();
    if( broadcast_notifier ){
      if( time >= Timeout_notify ){
        Timeout_notify = time + delay_ms_notify;
        Serial.printf( "[SEND] %s\n", remote_name );
        send();
      }
      return;
    }
    
    bool Connected_act = time < Timeout_disconnect;

    Change = (Connected_act != Connected);
    Connected = Connected_act;
    
    if( Connected ){
      if( Change ){
        Serial.printf( "[RISE] %s\n", remote_name );
      }
      if( time >= Timeout_notify ){
        Timeout_notify = time + delay_ms_notify;
        Serial.printf( "[SEND] %s\n", remote_name );
        send();
      }
    }else{
      if( Change ){
        Serial.printf( "[FALL] %s\n", remote_name );
      }
    }

  }

  void update_recive(){
    uint32_t time = millis();
    Timeout_disconnect = time + waiting_ms_disconnect;
    Change = !Connected;
    Connected = true;
  }

};

ESPNOW_device_connection notifier;
uint8_t espnow_device_connection_counter_max = 1;
uint8_t espnow_device_connection_counter = 0;
ESPNOW_device_connection espnow_device_connections[10];


// ===============================================================================
// External callback functions
// ===============================================================================
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len){

  Serial.printf( "\n\n-> recive: %s [%d]\n", mac2str(mac).c_str(), len );

  espnow_device_frame_t *pack = (espnow_device_frame_t*) data;
  
  if( pack->code != ESPNOW_DEVICE_CODE ) return;

  // update all connections
  for(int i=0;i<espnow_device_connection_counter;i++){
    if( strcmp( espnow_device_connections[ i ].remote_name, pack->name ) == 0 ){

      Serial.printf( "[name: %s connection: %d]\n", espnow_device_connections[ i ].remote_name, i );

      if( !espnow_device_local_server ){
        if( espnow_device_connections[ i ].searching_mac ){
          Serial.printf( "[found server MAC]\n", i );
          espnow_device_connections[ i ].set_mac( mac );
        }
      }
      
      if( memcmp( espnow_device_connections[ i ].remote_mac, mac, 6 ) != 0 ) return;
      espnow_device_connections[ i ].update_recive();
      espnow_device_connections[ i ].frame_in = *pack;

      return;
    }
  }

  if( espnow_device_local_server && espnow_device_connection_counter<espnow_device_connection_counter_max ){
    // [ decode aqui ]
    if( strncmp(pack->name_rx, espnow_device_local_name, ESPNOW_DEVICE_NAME_SIZE ) == 0 ){
      espnow_device_connections[ espnow_device_connection_counter ].connect_client( pack->name, mac);
      espnow_device_connections[ espnow_device_connection_counter ].update_recive();
      espnow_device_connections[ espnow_device_connection_counter ].frame_in = *pack;
      Serial.printf( "[name: %s connection: %d]\n", espnow_device_connections[ espnow_device_connection_counter ].remote_name, espnow_device_connection_counter );
      espnow_device_connection_counter++;
    }
  }

}


// ===============================================================================
// LOCAL DEVICE
// ===============================================================================

bool espnow_device_connected_led = false;

// ------------------
// connect
// ------------------

bool espnow_device_connect( const char * _name, const char *_password, const uint8_t *mac ){
  if( espnow_device_local_server ) return false;
  if( espnow_device_connection_counter >= espnow_device_connection_counter_max ) return false;
  espnow_device_connections[espnow_device_connection_counter].connect_server( _name, _password );
  espnow_device_connections[espnow_device_connection_counter].set_mac( mac );
  espnow_device_connection_counter++;
  return true;
}
  
bool espnow_device_connect( const char * _name, const char *_password = "" ){
  if( espnow_device_local_server ) return false;
  if( espnow_device_connection_counter >= espnow_device_connection_counter_max ) return false;
  espnow_device_connections[espnow_device_connection_counter].connect_server( _name, _password );
  espnow_device_connection_counter++;
  return true;
}

// ------------------
// begin / init
// ------------------

bool espnow_device_init(){

  espnow_device_connected_led = false;

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  //ESP_ERROR_CHECK( esp_wifi_set_channel(espnow_device_channel,WIFI_SECOND_CHAN_NONE) );
  if(esp_now_init() != 0){
    Serial.println("[Error] initializing ESP-NOW");
    return false;
  }
  Serial.println("\n\nBEGIN ESPNOW!!");

  #ifndef ESP32
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  #endif

  // callback espnow
  esp_now_register_recv_cb(espnow_device_recive);

  // diagnostico
  uint8_t primary;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);
  Serial.printf(  "[ESPNOW] channel: %d\t%d\n",primary,second);
  Serial.print(   "[ESPNOW] MAC:  ");Serial.println(WiFi.macAddress());
  Serial.println( "[ESPNOW Device] begin!" );

  return true;
}

void espnow_device_begin_server( const char * name = "SERVER", const char * password = "banana" ){
  espnow_device_local_server = true;
  strncpy( espnow_device_local_name, name, ESPNOW_DEVICE_NAME_SIZE );
  espnow_device_local_password = password;
  espnow_device_init();
  // notifier
  notifier.connect_client( espnow_device_local_name, espnow_device_broadcast_mac );
}

void espnow_device_begin_client( const char * name = "CLIENT" ){
  espnow_device_local_server = false;
  strncpy( espnow_device_local_name, name, ESPNOW_DEVICE_NAME_SIZE );
  espnow_device_init();
}

// ------------------
// update
// ------------------

void espnow_device_update(){

  bool led_act = false;

  // update all connections
  for(int i=0;i<espnow_device_connection_counter;i++){
    espnow_device_connections[ i ].update();
    if( !espnow_device_connections[ i ].Connected ){
      if( espnow_device_local_server ){
        // deleta a conexão e realoca as outras
        for(int j=i+1;j<espnow_device_connection_counter;j++)
          espnow_device_connections[j-1] = espnow_device_connections[j];
        i--; // o atual é deletado necessitando reiniciar a leitura
        espnow_device_connection_counter--;
      }else{
        espnow_device_connections[ i ].searching_mac = true;
      }
    }else{
      led_act = true;
    }
  }

  if( espnow_device_local_server ){
    if( espnow_device_connection_counter < espnow_device_connection_counter_max ){
      notifier.update(true);
    }
  }

  if( led_act != espnow_device_connected_led ){
    espnow_device_connected_led = led_act;
    digitalWrite(2,led_act);
  }

}