//========================================================================================
// ESPNOW Device V1
// Client e Server
//========================================================================================

#ifdef ESP32
  #include <esp_mac.h>
  #include <esp_now.h>
  #include <WiFi.h>
  #include <esp_wifi.h>
#elif defined(ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#endif

//========================================================================================
// Frame do protocolo
//========================================================================================

#define ESPNOW_DEVICE__CODE            18555 //2108 // codigo de identificação do protocolo
#define ESPNOW_DEVICE__NAME_SIZE       16
#define ESPNOW_DEVICE__BODY_SIZE       210
#define ESPNOW_DEVICE__MIN_SIZE        255 - ESPNOW_DEVICE__BODY_SIZE
#define ESPNOW_DEVICE__MAX_CONNECTIONS 10

typedef struct{
  // HEADER CODE [ 2 Bytes ] --------------------------------------------------------------
  uint16_t code        = ESPNOW_DEVICE__CODE; // codigo de identificação       [  2 Bytes ]
  // HEADER [ 22 Bytes ] ------------------------------------------------------------------
  char     name[ESPNOW_DEVICE__NAME_SIZE] = "ROBOT";  // Nome do dispositivo origem    [ 15 Bytes ]
  // HEADER LOCK [ 19 Bytes ] -------------------------------------------------------------
  uint16_t random      = 0;                  // Dados de segurança da conexão [  2 Bytes ]
  char     name_rx[ESPNOW_DEVICE__NAME_SIZE] = "RX"; // Dispositivo destino    [ 15 Bytes ]
  // Body (carga util) [ 214 Bytes ] ------------------------------------------------------
  uint16_t service     = 0;                  // identificador do serviço      [  2 Bytes ]
  uint8_t  len         = 0;                  // tamanho da carga              [ *2 Bytes ]
  uint8_t  body[ESPNOW_DEVICE__BODY_SIZE]; //                               [ 0-200 Bytes ]
}espnow_device_frame_t;

//========================================================================================
// Variaveis globais
//========================================================================================

uint8_t espnow_device_channel = 1;
const uint8_t espnow_device_broadcast_mac[6]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
char espnow_device_name[ESPNOW_DEVICE__NAME_SIZE] = "";
String espnow_device_password = "";

//========================================================================================
// ESPNOW Functions
//========================================================================================

#ifdef ESP32
typedef struct{
  bool set = false;
  esp_now_peer_info_t info;
}espnow_device_peer_t;

espnow_device_peer_t peerInfo[ESPNOW_DEVICE__MAX_CONNECTIONS];

espnow_device_peer_t *espnow_device_get_peer( uint8_t *mac ){
  for(int i=0;i<ESPNOW_DEVICE__MAX_CONNECTIONS;i++){
    if( peerInfo[i].set ){
      if( memcmp( peerInfo[i].info.peer_addr, mac, 6 ) == 0 ){
        return (peerInfo+i);
      }
    }
  }
  return nullptr;
}
#endif


String mac2str(const uint8_t *mac ){
  char char_str[18];
  snprintf(char_str, sizeof(char_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return char_str;
}

bool  espnow_device_peer( const uint8_t *mac ){

  #ifdef ESP32

    espnow_device_peer_t *p = nullptr;

    for(int i=0;i<ESPNOW_DEVICE__MAX_CONNECTIONS;i++){
      if( !peerInfo[i].set ) p = peerInfo+i;
    }

    if( p == nullptr ) return false;

    p->set = true;
    //peerInfo->ifidx = (wifi_interface_t) 0;
    memcpy(p->info.peer_addr, mac, 6);
    p->info.channel = espnow_device_channel;
    p->info.encrypt = false;

    // Add peer
    if(esp_now_add_peer(&p->info) != ESP_OK){
      Serial.println("[ESPNOW DEVICE] Failed to add peer");
      return false;
    }

  #else
    esp_now_add_peer( (uint8_t*) mac, ESP_NOW_ROLE_COMBO, espnow_device_channel, NULL, 0);
  #endif

  return true;
}

bool espnow_device_close_peer( uint8_t *mac ){
  //Serial.printf( "[CLOSE][%s]\n", mac2str(mac).c_str() );
  #ifdef ESP32
    espnow_device_peer_t *p = espnow_device_get_peer( mac );
    if( p == nullptr ) return false;
    esp_now_del_peer( p->info.peer_addr );
    p->set = false;
  #endif
  return false;
}

// Ler o canal Wi-Fi
uint8_t espnow_device_get_channel(){
  #ifdef ESP32
    uint8_t primary;
    wifi_second_chan_t channel_info;
    esp_wifi_get_channel(&primary, &channel_info);
    return primary;
  #elif defined( ESP8266 )
    return WiFi.channel();
  #endif
}

//========================================================================================
// ESPNOW Device Events
//========================================================================================

enum espnow_device_event_t{
  ESPNOW_EVT_NULL = 0,           // nenhuma ação
  //ESPNOW_EVT_BIND,               // 
  ESPNOW_EVT_SEND,               // [update ou asincrono] chamada assincrona ou timer write
  ESPNOW_EVT_RECIVE,             // [callback] recive
  ESPNOW_EVT_NOTIFY,             // [update  ] Notify
  ESPNOW_EVT_CONNECTED,          // [callback] recive
  ESPNOW_EVT_DISCONNECTED,       // [update  ] update
  ESPNOW_EVT_SCAN_FOUND
  //ESPNOW_EVT_CONNECTION_REQUEST  // [callback] recive
};


// callback functions
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len);
#elif defined( ESP8266 )
void espnow_device_recive( uint8_t * mac, uint8_t *data, uint8_t len);
#endif

//========================================================================================
// ESPNOW RADIO
//========================================================================================

class ESPNOW_DEVICE{
  
  bool Init = false;

  public:

    //----------------------------------------------------------------------------------------
    // Local device configurations
    //----------------------------------------------------------------------------------------
    bool Server = false;
    uint8_t simultaneous_connections = 1; // quantidade de conexões simultaneas
    bool LOG = true;
    
    bool auto_send = false;
    uint32_t send_delay = 50;
    
    bool auto_disconnect = false;
    uint32_t disconnect_delay = 1000;

    //----------------------------------------------------------------------------------------
    // begin
    //----------------------------------------------------------------------------------------

    void begin_server( const char * name = "SERVER", const char * password = "banana", uint8_t Simultaneous_connections = 1, uint32_t _send_delay = 300, uint32_t _disconnect_delay = 1000 ){
      Server = true;

      simultaneous_connections = Simultaneous_connections;

      // Automatic send
      auto_send = ( _send_delay > 0 );
      send_delay = _send_delay;

      // Automatic disconnect
      auto_disconnect = ( _disconnect_delay > 0 );
      disconnect_delay = _disconnect_delay;

      // copia os dados
      strncpy( espnow_device_name, name, ESPNOW_DEVICE__NAME_SIZE );
      strncpy( frame_out.name, name, ESPNOW_DEVICE__NAME_SIZE );
      espnow_device_password = password;

      // inicia
      init();
    }

    void begin_client( const char * name = "CLIENT", uint8_t Simultaneous_connections = 1, uint32_t _send_delay = 50, uint32_t _disconnect_delay = 0 ){
      Server = false;

      simultaneous_connections = Simultaneous_connections;

      auto_send = ( _send_delay > 0 );
      send_delay = _send_delay;

      auto_disconnect = ( _disconnect_delay > 0 );
      disconnect_delay = _disconnect_delay;

      // copia os dados
      strncpy( espnow_device_name, name, ESPNOW_DEVICE__NAME_SIZE );
      strncpy( frame_out.name, name, ESPNOW_DEVICE__NAME_SIZE );
      
      // inicia
      init();
    }


    //----------------------------------------------------------------------------------------
    // connexões
    //----------------------------------------------------------------------------------------

    typedef struct{
      bool     connected = false;
      bool     recived = false;
      uint8_t  mac[8];
      char     name[ESPNOW_DEVICE__NAME_SIZE];
      String   password;
      uint32_t last_time_send = 0;
      uint32_t last_time_recive = 0;
      espnow_device_frame_t frame;
    }connection_t;

    espnow_device_frame_t frame_out;

    uint8_t connections_count = 0; // quantidade de conexões abertas
    connection_t broadcast;
    connection_t Connections[ESPNOW_DEVICE__MAX_CONNECTIONS];

    connection_t *connection( const char *name ){
      for(int i=0;i<connections_count;i++){
        if( strcmp( Connections[i].name, name ) == 0 ) return (Connections+i);
      }
      return nullptr;
    }

    connection_t *connection_index( int i ){
      return ( i>=0 && i<connections_count ? (Connections+i) : &broadcast );
      return nullptr;
    }


    //----------------------------------------------------------------------------------------
    // lista de connexões < possiveis dispositivos para conectar >
    // apenas para clients
    //----------------------------------------------------------------------------------------

    typedef struct{
      char    name[ESPNOW_DEVICE__NAME_SIZE] = "";
      String  password = "";
      uint8_t mac[8];
    }espnow_device_t;

    #define CONNECTION_LIST_SIZE 20

    espnow_device_t connection_list[CONNECTION_LIST_SIZE];
    uint8_t connection_list_count = 0;

    bool connection_list_add( const char *name, const char *password ){
      if( connection_list_count >= CONNECTION_LIST_SIZE ) return false;
      strncpy( connection_list[ connection_list_count ].name, name, ESPNOW_DEVICE__NAME_SIZE );
      connection_list[ connection_list_count ].password = password;
      connection_list_count++;
      return true;
    }


    //----------------------------------------------------------------------------------------
    // INIT
    //----------------------------------------------------------------------------------------
    bool init(){
      
      close_all_connections();
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      //ESP_ERROR_CHECK( esp_wifi_set_channel(espnow_device_channel,WIFI_SECOND_CHAN_NONE) );
      if(esp_now_init() != 0){
        Serial.println( "[ESPNOW Device] Error initializing ESP-NOW!" );
        return false;
      }

      #ifdef ESP8266
      esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      #endif

      // callback espnow
      esp_now_register_recv_cb(espnow_device_recive);

      // diagnostico
      Serial.println( "[ESPNOW Device] init!" );
      Serial.printf( "[Local %s][ %s ][MAC: %s][Channel: %d]\n", Server ? "Server" : "Client", espnow_device_name, WiFi.macAddress(), espnow_device_get_channel() );

      //inicia a conexão broadcast

      // copia as credenciais
      strncpy( broadcast.name, "Broadcast", ESPNOW_DEVICE__NAME_SIZE );
      memcpy( broadcast.mac, espnow_device_broadcast_mac, 6 );
      Connections[connections_count].connected = true;
      espnow_device_peer( broadcast.mac );

      Init = true;

      return true;
    }

    // deinit
    void deinit(){
      Init = false;
      esp_now_deinit();
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      close_all_connections();
    }

    //----------------------------------------------------------------------------------------
    // send
    //----------------------------------------------------------------------------------------

    void send_connection( int i ){
      connection_t *p = connection_index(i);
      call( p == &broadcast ? ESPNOW_EVT_NOTIFY : ESPNOW_EVT_SEND, i );
      strncpy( frame_out.name_rx, p->name, ESPNOW_DEVICE__NAME_SIZE );
      esp_now_send( p->mac, (uint8_t*)&frame_out, constrain( ESPNOW_DEVICE__MIN_SIZE + frame_out.len, 0, 255 ) );
      p->last_time_send = millis();
    }

    //----------------------------------------------------------------------------------------
    // loop
    //----------------------------------------------------------------------------------------

    void loop(){

      if( !Init ) return;
      
      for(int i=0;i<connections_count;i++){
        if( auto_send ){
          cli();
          if( (millis() - Connections[i].last_time_send) >= send_delay ){
            send_connection( i );
          }
          sei();
        }

        if( auto_disconnect ){
          cli();
          if( (millis() - Connections[i].last_time_recive) >= disconnect_delay ){
            //Serial.printf( "last recive time: %d\n", millis() - Connections[i].last_time_recive );
            close_connection( Connections[i].name );
          }
          sei();
        }

      }
      
      if( Server ){
        if( connections_count < simultaneous_connections ){
          cli();
          if( (millis() - broadcast.last_time_send) >= send_delay ){
            send_connection(-1);
          }
          sei();
        }
      }

    }
    
    //----------------------------------------------------------------------------------------
    // Handler events
    //----------------------------------------------------------------------------------------
    void (*handle)(espnow_device_event_t,int) = nullptr;
    void set_handle_function(void (*f)(espnow_device_event_t,int)){ handle = f; }
    void call( espnow_device_event_t EVT, int id ){ if( handle != nullptr ) handle(EVT,id);  }


    //----------------------------------------------------------------------------------------
    // begin_connection -> inicia um objeto connection com os dados passados
    // close_connection
    // close_all_connections
    //----------------------------------------------------------------------------------------

    bool close_all_connections(){
      for(int i=0;i<connections_count;i++){
        espnow_device_close_peer( Connections[i].mac );
      }
      connections_count = 0;
      return true;
    }

    bool close_connection( const char * device_name ){
      // [!] seria bom interromper interrupt enquanto isso
      // caso a conexão ja esteja estabelecida
      connection_t *p = connection(device_name);
      if( p ){
        int index = p - Connections;
        //Serial.printf( "[DEVICE][close connection][%s] last recive time: %d ms\n", device_name, millis() - p->last_time_recive );
        call( ESPNOW_EVT_DISCONNECTED, index );
        espnow_device_close_peer( p->mac );
        for(int i=index+1;i<connections_count;i++){
          Connections[i-1] = Connections[i];
        }
        connections_count--;
        return true;
      }
      return false;
    }

    bool connect( const char * name, const uint8_t *mac ){

      if( connections_count >= simultaneous_connections ) return false;

      // copia as credenciais
      strncpy( Connections[connections_count].name, name, ESPNOW_DEVICE__NAME_SIZE );
      memcpy( Connections[connections_count].mac, mac, 6 );
      Connections[connections_count].last_time_recive = millis();
      Connections[connections_count].connected = true;
      Connections[connections_count].recived = true;
      //memcpy( (uint8_t*) &Connections[connections_count].frame, data, len );
      //Connections[connections_count].frame.len = len-ESPNOW_DEVICE__MIN_SIZE;
      
      espnow_device_peer( mac );

      connections_count++;

      // callback
      call( ESPNOW_EVT_CONNECTED, connections_count-1 );

      return true;
    }

    bool connect( const uint8_t *mac, espnow_device_frame_t * pack, uint8_t len ){

      if( connections_count >= simultaneous_connections ) return false;

      // copia as credenciais
      strncpy( Connections[connections_count].name, pack->name, ESPNOW_DEVICE__NAME_SIZE );
      memcpy( Connections[connections_count].mac, mac, 6 );
      Connections[connections_count].last_time_recive = millis();
      Connections[connections_count].connected = true;
      Connections[connections_count].recived = true;
      memcpy( (uint8_t*) &Connections[connections_count].frame, (uint8_t*) pack, len );
      Connections[connections_count].frame.len = len-ESPNOW_DEVICE__MIN_SIZE;
      
      espnow_device_peer( mac );

      connections_count++;

      // callback
      call( ESPNOW_EVT_CONNECTED, connections_count-1 );

      return true;
    }

    // ------------------------------------------------------------------------------------
    // scan
    // ------------------------------------------------------------------------------------
    bool scanning = false;
    void enable_scan(){
      scanning = true;
    }

    void disable_scan(){
      scanning = false;
    }
    

    // ------------------------------------------------------------------------------------
    // callback
    // ------------------------------------------------------------------------------------
    void recive(const uint8_t * mac, const uint8_t *data, int len){

      if( !Init ) return;
      if( len < ESPNOW_DEVICE__MIN_SIZE ) return;
      
      // logging
      //Serial.printf( "\n\n->[DEVICE][recive][%s][%d]\n", mac2str(mac).c_str(), len );

      // converte para o formato do protocolo
      #ifdef ESP8266
        espnow_device_frame_t _pack;
        espnow_device_frame_t *pack = &_pack;
        memcpy( (uint8_t*)pack, data, len );
      #else
        espnow_device_frame_t *pack = (espnow_device_frame_t*) data;
      #endif

      // logging
      //Serial.printf( "[DEVICE][RECIVE][Frame][name: %s][code: %d ][connection counter: %d ]\n", pack->name, pack->code, connections_count );

      // verifica o codigo de identificação do protocolo
      if( pack->code != ESPNOW_DEVICE__CODE ) return;

      // caso a conexão ja esteja estabelecida
      connection_t *p = connection(pack->name);
      if( p ){
        //Serial.println( "[DEVICE][RECIVE][Found Connection]" );
        if( memcmp( p->mac, mac, 6 ) != 0 ) return;
        //Serial.println( "[MAC OK!]" );
        if( strcmp( pack->name_rx, espnow_device_name ) != 0 && strcmp( pack->name_rx, "Broadcast" ) != 0 ) return;
        //Serial.println( "[NAME OK!]" );
        
        // Atualiza os dados da conexão
        p->last_time_recive = millis();
        memcpy( (uint8_t*) &p->frame, data, len );
        p->frame.len = len-ESPNOW_DEVICE__MIN_SIZE;
        //Serial.println( "[OK]" );
        p->recived = true;

        // chama callback
        int index = p - Connections;
        call( ESPNOW_EVT_RECIVE, index );
      }
      // caso a conexão ainda não esteja estabelecida
      else{

        //Serial.println( "[x]");

        if( connections_count >= simultaneous_connections ) return;
        
        if( Server ){ // Server
          if( strcmp( pack->name_rx, espnow_device_name ) != 0 ) return;
          
          // copia as credenciais
          strncpy( Connections[connections_count].name, pack->name, ESPNOW_DEVICE__NAME_SIZE );
          memcpy( Connections[connections_count].mac, mac, 6 );
          Connections[connections_count].last_time_recive = millis();
          Connections[connections_count].connected = true;
          Connections[connections_count].recived = true;
          memcpy( (uint8_t*) &Connections[connections_count].frame, data, len );
          Connections[connections_count].frame.len = len-ESPNOW_DEVICE__MIN_SIZE;
          
          espnow_device_peer( mac );

          connections_count++;

          // callback
          call( ESPNOW_EVT_CONNECTED, connections_count-1 );

        }else{ // Client
        
          // verifica se o nome do dispositivo consta na lista de connexões
          // não importa o target_name
          // problema -> se o server estiver se comunicando em broadcast...
          for( int i=0; i<connection_list_count; i++ ){
            if( strcmp( connection_list[i].name, pack->name ) == 0 ){
              connect( mac, pack, len );
            }
          }

          if( scanning ){
            if( strcmp( pack->name_rx, "Broadcast" ) == 0 ){
              connection_list_add( pack->name, "" );
              connect( mac, pack, len );
              disable_scan();
              call( ESPNOW_EVT_SCAN_FOUND, connection_list_count-1 );
            }
          }

        }
      }
    }
    
};

ESPNOW_DEVICE ESPNOW_device;

// ===============================================================================
// External callback functions
// ===============================================================================
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len){
  ESPNOW_device.recive( mac, data, len );
}
#elif defined( ESP8266 )
void espnow_device_recive( uint8_t * mac, uint8_t *data, uint8_t len){
  ESPNOW_device.recive( mac, data, len );
}
#endif