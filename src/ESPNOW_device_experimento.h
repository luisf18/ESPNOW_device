#ifndef ESPNOW_DEVICE_H
#define ESPNOW_DEVICE_H

//========================================================================================
// ESPNOW Device V2.0
// Client e Server
//========================================================================================
#include "ESPNOW_basis.h"
#include "ESPNOW_device_services.h"

//========================================================================================
// Frame do protocolo
//========================================================================================

#define ESPNOW_DEVICE__CODE            18555 //2108 // codigo de identificação do protocolo
#define ESPNOW_DEVICE__NAME_SIZE       16
#define ESPNOW_DEVICE__BODY_SIZE       210
#define ESPNOW_DEVICE__MIN_SIZE        255 - ESPNOW_DEVICE__BODY_SIZE

enum espnow_device_event_t{
  ESPNOW_DEVICE__EVT_NULL = 0,           // nenhuma ação
  ESPNOW_DEVICE__EVT_SEND,               // [update ou asincrono] chamada assincrona ou timer write
  ESPNOW_DEVICE__EVT_RECIVE,             // [callback] recive
  ESPNOW_DEVICE__EVT_NOTIFY,             // [update  ] Notify
  ESPNOW_DEVICE__EVT_CONNECTED,          // [callback] recive
  ESPNOW_DEVICE__EVT_DISCONNECTED,       // [update  ] update
  ESPNOW_DEVICE__EVT_SCAN_FOUND
};

//========================================================================================
// Struct
//========================================================================================

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
// Callback ESPNOW
//========================================================================================
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len);
#elif defined( ESP8266 )
void espnow_device_recive( uint8_t * mac, uint8_t *data, uint8_t len);
#endif

struct ESPNOW_device_t{
  uint8_t  mac[8];
  char     name[ESPNOW_DEVICE__NAME_SIZE];
  char     password[16];
  void send();
  //void begin( const char *_name, const char *_password, const uint8_t *_mac = 0 );
  //void send();
  //void update_recive( espnow_device_frame_t *_frame );
  void connect();
  void connect( const uint8_t *mac );
  void disconnect();
  //bool auto_send();
  //bool auto_disconnect();
  //void check_mac( const uint8_t *_mac );
  //void check_name( const char *_name )
}

class ESPNOW_device{
  public:
  cont char *name;
  connections_t connections[...];
  void begin();
  void send();
  void recive();
  void loop();
};

class ESPNOW_local_server : public ESPNOW_device  {
  public:
  cont char *password;
  void send(); // ...
  void recive(); // ...
};

class ESPNOW_local_client : public ESPNOW_device  {
  public:
  ESPNOW_device_t servers[...]; // nome e senha
  void send(); // ...
  void recive(); // ...
};

class ESPNOW_remote_server : public ESPNOW_device  {
  public:
  void send(); // ...
  void recive(); // ...
};

class ESPNOW_remote_client : public ESPNOW_device  {
  public:
  void send(); // ...
  void recive(); // ...
};


//========================================================================================
// Connection Class
//========================================================================================

class ESPNOW_connection{
  public:
  bool     connected = false;
  bool     recived = false;
  uint8_t  mac[8];
  char     name[ESPNOW_DEVICE__NAME_SIZE];
  char     password[16];
  uint32_t last_time_send = 0;
  uint32_t last_time_recive = 0;
  espnow_device_frame_t frame;
  //void begin( const char *_name, const char *_password, const uint8_t *_mac = 0 );
  //void send();
  //void update_recive( espnow_device_frame_t *_frame );
  void connect();
  void connect( const uint8_t *mac );
  void disconnect();
  //bool auto_send();
  //bool auto_disconnect();
  //void check_mac( const uint8_t *_mac );
  //void check_name( const char *_name )
};

//========================================================================================
// ESPNOW DEVICE
//========================================================================================

class ESPNOW_DEVICE{
  
  bool Init = false;

  public:

    bool      Server = false;
    char      name[16] = "";
    char      password[16] = "";
    uint8_t   connection_simultaneous = 1; // quantidade de conexões simultaneas
    uint8_t   connection_count = 0; // contagem de conexões abertas
    bool      Notify = false;
    uint32_t  send_delay = 50;
    bool      auto_disconnect = false;
    uint32_t  disconnect_delay = 1000;
    bool      LOG = true;

    //----------------------------------------------------------------------------------------
    // Handler events
    //----------------------------------------------------------------------------------------
    //void (*handle)(espnow_device_event_t,int) = nullptr;
    //void set_handle_function(void (*f)(espnow_device_event_t,int)){ handle = f; }
    //void call( espnow_device_event_t EVT, int id ){ if( handle != nullptr ) handle(EVT,id);  }
    
    void (*handle)(espnow_device_event_t,ESPNOW_connection*) = nullptr;
    void set_handle_function(void (*f)(espnow_device_event_t,ESPNOW_connection*)){ handle = f; }
    void call( espnow_device_event_t EVT, ESPNOW_connection* cnn ){ if( handle != nullptr ) handle(EVT,cnn);  }

    //----------------------------------------------------------------------------------------
    // connexões
    //----------------------------------------------------------------------------------------

    ESPNOW_connection connections[ESPNOW_DEVICE__MAX_CONNECTIONS];
    ESPNOW_connection broadcast;

    //----------------------------------------------------------------------------------------
    // begin
    //----------------------------------------------------------------------------------------

    void begin_server( const char * _name = "SERVER", const char * _password = "banana", uint8_t Simultaneous_connections = 1, uint32_t _send_delay = 300, uint32_t _disconnect_delay = 1000 ){
      Server = true;
      connection_simultaneous = Simultaneous_connections;
      Notify = true;
      send_delay = _send_delay;
      disconnect_delay = _disconnect_delay;

      // copia os dados
      strncpy( name, _name, ESPNOW_DEVICE__NAME_SIZE );
      strncpy( password, _password, 16 );

      init();
    }

    void begin_client( const char * _name = "CLIENT", uint8_t Simultaneous_connections = 1, uint32_t _send_delay = 50, uint32_t _disconnect_delay = 0 ){
      Server = false;
      connection_simultaneous = Simultaneous_connections;
      Notify = false;
      send_delay = _send_delay;
      disconnect_delay = _disconnect_delay;
      strncpy( name, _name, ESPNOW_DEVICE__NAME_SIZE );
      // inicia
      init();
    }

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

      // diagnostico
      Serial.println( "[ESPNOW Device] init!" );
      Serial.printf( "[Local %s][ %s ][MAC: %s][Channel: %d]\n", Server ? "Server" : "Client", name, WiFi.macAddress(), espnow_device_get_channel() );

      // callback espnow
      esp_now_register_recv_cb(espnow_device_recive);

      // connecta ao broadcast
      if( Notify ){
        broadcast.connect();
      }

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
    // gerenciador de listas de conexões
    //----------------------------------------------------------------------------------------

    bool available_connection(){
      for(int i=0;i<)
    }

    bool add_connection( const char *_name, const char *_password ){

    }

    //----------------------------------------------------------------------------------------
    // loop
    //----------------------------------------------------------------------------------------

    void loop(){
      if( !Init ) return;
      /*/
      if( connection_count ){
        for( int i=0; i<connection_count; i++ ){
          if( connections[i].connected ){
            if( connections[i].auto_disconnect() ){
              call( ESPNOW_DEVICE__EVT_DISCONNECTED, i );
            }else if( connections[i].time_to_send() ){
              call( ESPNOW_DEVICE__EVT_SEND, i );
              connections[i].send();
            }
          }
        }
      }else{
        if( Notify ){
          if( broadcast.time_to_send() ){
            call( ESPNOW_DEVICE__EVT_NOTIFY, -1 );
            broadcast.send();
          }
        }
      }
      /*/
    }

    //----------------------------------------------------------------------------------------
    // begin_connection -> inicia um objeto connection com os dados passados
    // close_connection
    // close_all_connections
    //----------------------------------------------------------------------------------------

    /*/
    static inline void check_name( const uint8_t *_name ){
      return ( strcmp( name, _name ) == 0 );
    }
    /*/

    bool close_all_connections(){
      for(int i=0;i<connection_count;i++) connections[i].disconnect();
      connection_count = 0;
      return true;
    }
    

    // ------------------------------------------------------------------------------------
    // callback
    // ------------------------------------------------------------------------------------
    void recive(const uint8_t * mac, const uint8_t *data, int len){
      
      /*/
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

      int i = search_connection(pack->name,Server);

      if( Server ){
        // caso a conexão ja esteja estabelecida
        if( i>=0 ){
          // achou uma conexão aberta
          if(!connections[i].check_mac( mac )) return;
          //decode( pack ); // decodifica a msg usando a senha local
          if(!check_name( pack->name_rx )) return;
          connections[i].update_recive( pack );
        }else{
          //decode( pack ); // decodifica a msg usando a senha local
          if(!check_name( pack->name_rx )) return;
          i = connection_available(); // busca uma conexão
          if( i<0 ) return;
          connections[i].begin( pack->name, password, mac );
          connections[i].connect();
          connections[i].update_recive( pack );
          call( ESPNOW_DEVICE__EVT_CONNECTED, i );
        }
      }else{
        // Client
        // busca conexões com esse nome
        int i = search_connection(pack->name,false);
        if( i<0 ) return;
        if( !connections[i].connected ){
          // tenta conectar
          if( connection_count >= connection_simultaneous ) return;
          connections[i].connect(mac);
          //decode( pack ); // decodifica a msg usando a senha do server
          connections[i].update_recive( pack );
        }
          // achou uma conexão na lista
          call( ESPNOW_DEVICE__EVT_CONNECTED, i );
        }
      }
      call( ESPNOW_DEVICE__EVT_RECIVE, i );
      /*/
    }
    
};

ESPNOW_DEVICE ESPNOW_device;

// ==================================================================================
// Connection Class Functions
// ==================================================================================

void ESPNOW_connection::connect(){
  cli();
  espnow_device_peer( mac );
  connected = true;
  sei();
}
void ESPNOW_connection::connect( const uint8_t *_mac ){
  memcpy( mac, _mac, 6 );
  connect();
}
void ESPNOW_connection::disconnect(){
  connected = false;
  espnow_device_close_peer( mac );
}
/*/
void ESPNOW_connection::send(){
  strncpy( ESPNOW_device.frame.name, ESPNOW_device.name, ESPNOW_DEVICE__NAME_SIZE );
  strncpy( ESPNOW_device.frame.name_rx, name, ESPNOW_DEVICE__NAME_SIZE );
  esp_now_send( mac, (uint8_t*)&ESPNOW_device.frame, constrain( ESPNOW_DEVICE__MIN_SIZE + ESPNOW_device.frame.len, 0, 250 ) );
  last_time_send = millis();
}
void ESPNOW_connection::update_recive( espnow_device_frame_t *_frame ){
  frame = *_frame;
  last_time_send = millis();
  recived = true;
}
void begin( const char *_name, const char *_password, const uint8_t *_mac = 0 ){
  if( _mac ) memcpy( mac, _mac, 6 );
  if( _password ) strcpy( password, _password, 16 );
  srtncpy( name, _name, ESPNOW_DEVICE__NAME_SIZE );
}
void ESPNOW_connection::check_mac( const uint8_t *_mac ){
  return ( memcmp( mac, _mac, 6 ) == 0 );
}
bool ESPNOW_connection::check( espnow_device_frame_t *_frame, const uint8_t *_mac ){
  if( !check_mac(_mac) ) return false;
  if( !check_mac(_frame) ) return false;
}
bool ESPNOW_connection::auto_send(){
  return ( (millis() - last_time_send) > ESPNOW_DEVICE.send_delay );
}
bool ESPNOW_connection::auto_disconnect(){
  if( (millis() - last_time_recive) > ESPNOW_DEVICE.disconnect_delay ){
    disconnect();
    return true;
  }
  return false;
}
bool ESPNOW_connection::check_name( const char *_name ){
  return ( strcmp( name, _name, 6 ) == 0 );
}
/*/

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

#endif