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
#define ESPNOW_DEVICE__MIN_SIZE        39

#define ESPNOW_DEVICE__CHECK_MAC( M1, M2 ) ( memcmp( M1, M2, 6 ) == 0 )
#define ESPNOW_DEVICE__CHECK_NAME( N1, N2 ) ( strcmp( N1, N2 ) == 0 )
#define ESPNOW_DEVICE__COPY_NAME( X, Y ){ strncpy( X, Y, ESPNOW_DEVICE__NAME_SIZE ); }
#define ESPNOW_DEVICE__COPY_MAC( X, Y ) { memcpy( X, Y, 6 ); }
#define ESPNOW_DEVICE__COPY_PASSWORD( X, Y ){ strncpy( X, Y, 16 ); }

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

// [ service, len, service, len, 

typedef struct __attribute__((packed)){
  // HEADER CODE [ 2 Bytes ] --------------------------------------------------------------
  uint16_t code        = ESPNOW_DEVICE__CODE; // codigo de identificação       [  2 Bytes ]
  // HEADER [ 22 Bytes ] ------------------------------------------------------------------
  char     name[ESPNOW_DEVICE__NAME_SIZE] = "ROBOT";  // Nome do dispositivo origem    [ 16 Bytes ]
  // HEADER LOCK [ 19 Bytes ] -------------------------------------------------------------
  uint16_t random      = 0;                  // Dados de segurança da conexão [  2 Bytes ]
  char     name_rx[ESPNOW_DEVICE__NAME_SIZE] = "RX"; // Dispositivo destino    [ 16 Bytes ]
  // Body (carga util) [ 214 Bytes ] ------------------------------------------------------
  uint16_t service     = 0;                  // identificador do serviço      [ 2 Bytes ]
  uint8_t  len         = 0;                  // tamanho da carga              [ 1 Byte  ]
  //uint8_t body[ESPNOW_DEVICE__BODY_SIZE];   // [ 0-200 Bytes ]
  union{
    uint8_t body[ESPNOW_DEVICE__BODY_SIZE];   // [ 0-210 Bytes ]
    int     ch[52];
    ESPNOW_SERVICE::radio_t radio;
  };
}espnow_device_frame_t;

//========================================================================================
// Callback ESPNOW
//========================================================================================
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len);
#elif defined( ESP8266 )
void espnow_device_recive( uint8_t * mac, uint8_t *data, uint8_t len);
#endif

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
  ESPNOW_connection(){}
  ESPNOW_connection(const char *_name, const char *_password, const uint8_t *_mac = nullptr){
    begin( _name, _password, _mac );
  }
  void begin( const char *_name, const char *_password, const uint8_t *_mac = nullptr );
  void send();
  void recive( espnow_device_frame_t *_frame );
  void connect();
  void connect( const uint8_t *mac );
  void disconnect();
  bool send_timeout();
  bool auto_disconnect();
  bool loop();
};

//typedef struct{
//  char name[ESPNOW_DEVICE__NAME_SIZE];
//  char password[16];
//  uint8_t mac[6];
//} ESPNOW_DEVICE_server_credentials_t;

//========================================================================================
// ESPNOW DEVICE
//========================================================================================

class ESPNOW_DEVICE{
  
  bool Init = false;

  public:

    // caracteristicas do device local
    bool      Server = false;
    char      name[16] = "";
    char      password[16] = "";
    bool      Notify = false;
    bool      Scanning = false;
    uint32_t  send_delay = 50;
    bool      auto_disconnect = false;
    uint32_t  disconnect_delay = 1000;
    uint32_t  client_init_connection_delay = 100;
    
    // conexão
    uint8_t   connection_simultaneous = 1; // quantidade de conexões simultaneas
    uint8_t   connection_count = 0; // contagem de conexões abertas
    uint8_t   connection_servers_len = 0; // contagem de conexões server salvas
    ESPNOW_connection connections[ESPNOW_DEVICE__MAX_CONNECTIONS];
    ESPNOW_connection broadcast = ESPNOW_connection( "Broadcast", 0, espnow_device__broadcast_mac );
    espnow_device_frame_t frame;
    //ESPNOW_connection (*connections_order)[ESPNOW_DEVICE__MAX_CONNECTIONS];
    
    // ### ESPNOW DEVICE logging level
    // - 0: no logging
    // - 1: init and begin log
    // - 2: (1) and connect/disconnect log
    // - 3: (2) and recive and send log
    uint8_t log_level = 1;

    // [futuro] adiconar futuramente uma lista extendida
    //   somente para o client: lista de servers que pode se conectar
    //uint8_t server_list_len = 0;
    //ESPNOW_DEVICE_server_credentials_t *server_list = nullptr;
    //bool set_server_list()

    //----------------------------------------------------------------------------------------
    // Handler events
    //----------------------------------------------------------------------------------------
    //void (*handle)(espnow_device_event_t,int) = nullptr;
    //void set_handle_function(void (*f)(espnow_device_event_t,int)){ handle = f; }
    //void call( espnow_device_event_t EVT, int id ){ if( handle != nullptr ) handle(EVT,id);  }
    
    void (*handle)(espnow_device_event_t,ESPNOW_connection*) = nullptr;
    void set_handler(void (*f)(espnow_device_event_t,ESPNOW_connection*)){ handle = f; }
    void call( espnow_device_event_t EVT, ESPNOW_connection* cnn ){ if( handle != nullptr ) handle(EVT,cnn);  }

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

    void begin_client( const char * _name = "CLIENT", uint8_t Simultaneous_connections = 1, uint32_t _send_delay = 50, uint32_t _disconnect_delay = 1000 ){
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
        if( log_level ){
          Serial.println( "[ESPNOW Device] Error initializing ESP-NOW!" );
        }
        return false;
      }

      #ifdef ESP8266
      esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      #endif

      // logging
      if( log_level ){
        Serial.println( "[ESPNOW Device] init!" );
        Serial.printf( "[Local %s][ %s ][MAC: %s][Channel: %d]\n", Server ? "Server" : "Client", name, WiFi.macAddress().c_str(), espnow_device_get_channel() );
      }

      // callback espnow
      esp_now_register_recv_cb(espnow_device_recive);

      // connecta ao broadcast
      //if( Notify ){
        broadcast.connect();
      //}

      Init = true;

      return true;
    }

    // deinit
    void deinit(){
      Init = false;
      close_all_connections();
      esp_now_deinit();
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
    }

    //----------------------------------------------------------------------------------------
    // adicionar servers (só pra clients)
    //----------------------------------------------------------------------------------------

    bool add_server( const char *_name, const char *_password ){
      if( connection_servers_len >= ESPNOW_DEVICE__MAX_CONNECTIONS ) return false;
      connections[connection_servers_len].begin( _name, _password );
      connection_servers_len++;
      return true;
    }

    //----------------------------------------------------------------------------------------
    // preparar a carga
    //----------------------------------------------------------------------------------------

    void load( uint16_t service, const uint8_t *p, uint8_t len ){
      frame.service = service;
      frame.len = constrain( len, 0, ESPNOW_DEVICE__BODY_SIZE );
      memcpy( frame.body, p, frame.len );
    }

    //----------------------------------------------------------------------------------------
    // gerenciador de listas de conexões
    //----------------------------------------------------------------------------------------

    void close_all_connections(){
      for(int i=0;i<connection_count;i++) connections[i].disconnect();
      connection_count = 0;
    }

    // only for server
    ESPNOW_connection *available_connection(){
      for(int i=0;i<connection_simultaneous;i++){
        if( !connections[i].connected ) return connections+i;
      }
      return nullptr;
    }

    ESPNOW_connection *search_client( const char *_name ){
      for(int i=0;i<connection_simultaneous;i++){
        if( !connections[i].connected ) continue;
        if( strcmp( connections[i].name, _name ) == 0 ) return connections+i;
      }
      return nullptr;
    }

    ESPNOW_connection *search_server( const char *_name ){
      for(int i=0;i<connection_servers_len;i++){
        if( strcmp( connections[i].name, _name ) == 0 ) return connections+i;
      }
      return nullptr;
    }

    void send( const uint8_t *_mac ){
      ESPNOW_DEVICE__COPY_NAME( frame.name, name );
      // logging
      if( log_level >= 3 ){
        Serial.printf(
          "--ESPNOW: [ send %s %s %d %d ]\n",
          mac2str(_mac).c_str(),
          frame.name_rx,
          frame.service,
          frame.len
        );
      }
      esp_now_send( _mac, (uint8_t*) &frame, constrain( ESPNOW_DEVICE__MIN_SIZE + frame.len, 0, 250 ) );
    }

    //ESPNOW_connection *search_connection( const char *_name, bool open_olny ){
    //  for(int i=0;i<ESPNOW_DEVICE__MAX_CONNECTIONS;i++){
    //    if( open_olny ){
    //      if( !connections[i].connected ) continue;
    //    }
    //    if( strcmp( connections[i].name, _name ) == 0 ) return connections+i;
    //  }
    //  return nullptr;
    //}

    //----------------------------------------------------------------------------------------
    // loop
    //----------------------------------------------------------------------------------------

    void loop(){
      if( !Init ) return;
      if( connection_count ){
        int n = connection_count;
        for( int i=0; i<n; i++ ){
          if(connections[i].loop()) connection_count--;
        }
      }else{
        if( Notify ){
          if( broadcast.send_timeout() ){
            call( ESPNOW_DEVICE__EVT_NOTIFY, &broadcast );
            broadcast.send();
          }
        }
      }
    }
    

    // ------------------------------------------------------------------------------------
    // callback
    // ------------------------------------------------------------------------------------
    void recive(const uint8_t * mac, const uint8_t *data, int len){
      if( !Init ) return;
      if( len < ESPNOW_DEVICE__MIN_SIZE ) return;

      // converte para o formato do protocolo
      #ifdef ESP8266
        espnow_device_frame_t _Frame;
        espnow_device_frame_t *Frame = &_Frame;
        memcpy( (uint8_t*)Frame, data, len );
      #else
        espnow_device_frame_t *Frame = (espnow_device_frame_t*) data;
      #endif

      // logging
      if( log_level >= 3 ){
        Serial.printf(
          "\n--ESPNOW: [ recive %s %s (%d) >> %s ][%d:%d][%d/%d]\n",
          mac2str(mac).c_str(),
          Frame->name,
          len,
          Frame->name_rx,
          Frame->code,
          Frame->service,
          connection_count,
          connection_simultaneous
        );
      }

      // verifica o codigo de identificação do protocolo
      if( Frame->code != ESPNOW_DEVICE__CODE ) return;

      //ESPNOW_connection *cnn = search_connection(Frame->name,Server);
      ESPNOW_connection *cnn = nullptr;

      if( Server ){
        cnn = search_client( Frame->name );
        // caso a conexão não esteja estabelecida
        if( !cnn ){ 
          //decode( pack ); // decodifica a msg usando a senha local
          if(!ESPNOW_DEVICE__CHECK_NAME( Frame->name_rx, name ) ) return;
          cnn = available_connection(); // busca uma conexão
          if( !cnn ) return;
          cnn->begin( Frame->name, password, mac ); // inicia uma conexão com client usando a senha local
          cnn->connect();
          connection_count++;
          cnn->recive( Frame );
          call( ESPNOW_DEVICE__EVT_CONNECTED, cnn );
          call( ESPNOW_DEVICE__EVT_RECIVE, cnn );
          return;
        }
      }else{
        // Client
        cnn = search_server( Frame->name );
        if( cnn ){
          if( !cnn->connected ){ // achou mas n esta conectado
            // tenta conectar
            if( connection_count >= connection_simultaneous ) return;
            cnn->connect(mac);
            connection_count++;
            //connections[i].decode( pack ); // decodifica a msg usando a senha do server
            cnn->recive( Frame );
            // achou uma conexão na lista
            call( ESPNOW_DEVICE__EVT_CONNECTED, cnn );
            call( ESPNOW_DEVICE__EVT_RECIVE, cnn );
            return;
          }
        }else{ // se não etiver na lista não connecta
          if( Scanning ){
            ESPNOW_connection temp;
            temp.frame = *Frame;
            temp.begin( Frame->name, 0, mac );
            call( ESPNOW_DEVICE__EVT_SCAN_FOUND, &temp );
          }
          return;
        }
      }

      // verifica o mac
      if( !ESPNOW_DEVICE__CHECK_MAC( cnn->mac, mac ) ) return;
      // verifica o destino
      if(!ESPNOW_DEVICE__CHECK_NAME( Frame->name_rx, name ) ) return;
      cnn->recive( Frame );
      call( ESPNOW_DEVICE__EVT_RECIVE, cnn );
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
  if( ESPNOW_device.log_level >= 2 ){
    Serial.printf( "--ESPNOW: [ connected %s ]\n", name );
  }
}
void ESPNOW_connection::connect( const uint8_t *_mac ){
  ESPNOW_DEVICE__COPY_MAC( mac, _mac );
  connect();
}
void ESPNOW_connection::disconnect(){
  cli();
  connected = false;
  espnow_device_close_peer( mac );
  uint32_t dt = (millis() - last_time_recive);
  sei();
  if( ESPNOW_device.log_level >= 2 ){
    Serial.printf( "--ESPNOW: [ disconnected %s - timeout = %lu ms]\n", name, dt );
  }
  ESPNOW_device.call( ESPNOW_DEVICE__EVT_DISCONNECTED, this );
}
void ESPNOW_connection::begin( const char *_name, const char *_password, const uint8_t *_mac ){
  if( _mac ) ESPNOW_DEVICE__COPY_MAC( mac, _mac );
  if( _password ) ESPNOW_DEVICE__COPY_PASSWORD( password, _password );
  ESPNOW_DEVICE__COPY_NAME( name, _name );
}
void ESPNOW_connection::recive( espnow_device_frame_t *_frame ){
  frame = *_frame;
  last_time_recive = millis();
  recived = true;
}
void ESPNOW_connection::send(){
  ESPNOW_DEVICE__COPY_NAME( ESPNOW_device.frame.name_rx, name );
  // encode( password )
  ESPNOW_device.send( mac );
  last_time_send = millis();
}
bool ESPNOW_connection::send_timeout(){
  return ( (millis() - last_time_send) > ESPNOW_device.send_delay );
}
bool ESPNOW_connection::auto_disconnect(){
  cli();
  uint32_t dt = (millis() - last_time_recive);
  sei();
  if( dt > ESPNOW_device.disconnect_delay ){
    disconnect();
    return true;
  }
  return false;
}

// só retorna true se desconectar
bool ESPNOW_connection::loop(){
  if( !connected ) return false;
  if( send_timeout() ){
    ESPNOW_device.call( ESPNOW_DEVICE__EVT_SEND, this );
    send();
  }
  return auto_disconnect();
}

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