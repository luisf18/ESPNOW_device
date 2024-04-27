#ifndef ESPNOW_DEVICE_H
#define ESPNOW_DEVICE_H

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
// variaveis e constantes
//========================================================================================

enum espnow_device_event_t{
  ESPNOW_EVT_NULL = 0,           // nenhuma ação
  ESPNOW_EVT_PUBLIC_NOTIFY,      // [update  ] Notify
  ESPNOW_EVT_NOTIFY,             // [update  ] Notify
  ESPNOW_EVT_SEND,               // [update ou asincrono] chamada assincrona ou timer write
  ESPNOW_EVT_RECIVE,             // [callback] recive
  ESPNOW_EVT_CONNECTED,          // [callback] recive
  ESPNOW_EVT_DISCONNECTED,       // [update  ] update
  ESPNOW_EVT_CONNECTION_REQUEST  // [callback] recive
};

const uint8_t espnow_device_broadcast_mac[6]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#define ESPNOW_DEVICE_CODE 18555
#define ESPNOW_DEVICE_BODY_SIZE 210
#define ESPNOW_DEVICE_NAME_SIZE 16
#define ESPNOW_DEVICE_CONNECTIONS_MAX 10

// credenciais
char   ESPNOW_DEVICE_NAME[ ESPNOW_DEVICE_NAME_SIZE ] = "SERVER";
String ESPNOW_DEVICE_PASSWORD                        = "1234";
bool   ESPNOW_DEVICE_SERVER                          = true;
bool   ESPNOW_DEVICE_LOG_DEBUG                       = false; // <- logging


//========================================================================================
// Struct
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
// basic functions
//========================================================================================

#ifdef ESP32
bool peer( const uint8_t *mac, esp_now_peer_info_t *peerInfo ){
    // Register peer device
    memcpy(peerInfo->peer_addr, mac, 6);
    //peerInfo->channel = espnow_device_channel;
    peerInfo->encrypt = false;
    // Add peer
    if (esp_now_add_peer(peerInfo) != ESP_OK){
      if( ESPNOW_DEVICE_LOG_DEBUG ){
        Serial.println("Failed to add peer");
      }
      return false;
    }
  return true;
}
#elif defined( ESP8266 )
bool peer( const uint8_t *mac ){
  uint8_t channel = 1;
  esp_now_add_peer( (uint8_t*) mac, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  return true;
}
#endif

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

/*/
// extraido do "espRadio.h"
// avaliar adição...
boolean del_peer( uint8_t *mac ){
  boolean ok = true;
  #if defined(ESP32)
    esp_err_t Status = esp_now_del_peer(mac);
    //log_err(Status);
    ok = (Status == ESP_OK || Status == ESP_ERR_ESPNOW_EXIST );
  #elif defined(ESP8266)
    esp_now_del_peer( mac );
  #endif
  return ok;
}
/*/

String mac2str(const uint8_t *mac ){
  char char_str[18];
  snprintf(char_str, sizeof(char_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return char_str;
}

// callback functions
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len);
#elif defined( ESP8266 )
void espnow_device_recive( uint8_t * mac, uint8_t *data, uint8_t len);
#endif


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
  uint32_t waiting_ms_disconnect = 2000;
  uint32_t Timeout_notify        = 0;
  uint32_t delay_ms_notify       = 1000;

  //----------------------------------------------------------------------------------------
  // State Flags
  //----------------------------------------------------------------------------------------
  boolean fall(){ return Change&&(!Connected); }
  boolean rise(){ return Change&&Connected; }
  boolean change(){ return Change; }
  boolean connected(){ return Connected; }

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

    // logging
    if( ESPNOW_DEVICE_LOG_DEBUG ){
      if( memcmp( mac, espnow_device_broadcast_mac, 6 ) == 0 ){
        Serial.println( ">> connect broadcast notifier!" );
      }else{
        Serial.printf( ">> connect to remote %s %s!\n", (ESPNOW_DEVICE_SERVER?"client":"server"), remote_name );
      }
    }

  }
  
  // conexão em um server
  void connect_server( const char *name, const char *_password ){

    // Remote info
    strncpy( remote_name, name, ESPNOW_DEVICE_NAME_SIZE );
    remote_password = _password;
    
    // frame header
    strncpy( frame_out.name, ESPNOW_DEVICE_NAME, ESPNOW_DEVICE_NAME_SIZE );
    strncpy( frame_out.name_rx, remote_name, ESPNOW_DEVICE_NAME_SIZE );

    // connection flags
    Change = false;
    Connected = false;
    searching_mac = true;

    if( ESPNOW_DEVICE_LOG_DEBUG ){
      Serial.printf( ">> request connection to server %s!\n", name);
    }

  }


  // um dispositivo client se connectou ao dispositivo local
  void connect_client( const char *name, const uint8_t *mac ){
    
    if( !ESPNOW_DEVICE_SERVER ) return;

    // Remote info
    strncpy( remote_name, name, ESPNOW_DEVICE_NAME_SIZE );
    
    // frame header
    strncpy( frame_out.name, ESPNOW_DEVICE_NAME, ESPNOW_DEVICE_NAME_SIZE );
    strncpy( frame_out.name_rx, remote_name, ESPNOW_DEVICE_NAME_SIZE );
    
    // connection flags
    update_recive();

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
    cli();
    esp_now_send( remote_mac, (uint8_t*)&frame_out, sizeof(frame_out) - (ESPNOW_DEVICE_BODY_SIZE-frame_out.len) );
    sei();
  }

  // ---------------------------------------------------------------------
  // loop
  // ---------------------------------------------------------------------

  bool update_flags( bool broadcast_notifier = false ){
    // broadcast notifier
    uint32_t time = millis();
    if( broadcast_notifier ){
      if( time >= Timeout_notify ){
        Timeout_notify = time + delay_ms_notify;
        return true;
      }
      return false;
    }
    
    bool notify = false;
    bool Connected_act = time < Timeout_disconnect;

    Change = (Connected_act != Connected);
    Connected = Connected_act;
    
    if( Connected ){
      if( time >= Timeout_notify ){
        notify = true;
        Timeout_notify = time + delay_ms_notify;
      }
    }

    return notify;
  }

  void update( bool broadcast_notifier = false ){
    if( update_flags( broadcast_notifier ) ) send();
  }

  void update_recive(){
    uint32_t time = millis();
    Timeout_disconnect = time + waiting_ms_disconnect;
    Change = !Connected;
    Connected = true;
  }

};


// ===============================================================================
// LOCAL DEVICE
// ===============================================================================

class ESPNOW_DEVICE{
  
  public:
    
    //----------------------------------------------------------------------------------------
    // connexões
    //----------------------------------------------------------------------------------------
    bool    espnow_device_init = false;
    ESPNOW_device_connection notifier;
    //bool    always_public_notify = false;
    uint8_t connections_counter_max = 1;
    uint8_t connections_counter = 0;
    ESPNOW_device_connection connections[ESPNOW_DEVICE_CONNECTIONS_MAX];
    bool    single_server = false;
    
    // timing
    uint32_t waiting_ms_disconnect = 500;
    uint32_t delay_ms_notify       = 50;

    uint8_t connections_made(){
      uint8_t count = 0;
      for(int i=0;i<connections_counter;i++){
        if( connections[i].connected() ) count++;
      }
      return count;
    }
    
    //----------------------------------------------------------------------------------------
    // LED
    //----------------------------------------------------------------------------------------
    bool connection_led_change   = false;
    bool connection_led          = false;
    int  connection_led_pin      = -1;
    bool connection_led_state_on = HIGH;
    
    void set_led( int pin, bool state_on = HIGH ){
      connection_led_pin = pin;
      connection_led_state_on = state_on;
    }

    //----------------------------------------------------------------------------------------
    // Handler events
    //----------------------------------------------------------------------------------------
    void (*handle)(espnow_device_event_t,int) = nullptr;
    void set_handle_function(void (*f)(espnow_device_event_t,int)){ handle = f; }
    void call( espnow_device_event_t EVT, int id ){ if( handle != nullptr ) handle(EVT,id);  }


    //----------------------------------------------------------------------------------------
    // INIT
    //----------------------------------------------------------------------------------------
    bool init(){
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      //ESP_ERROR_CHECK( esp_wifi_set_channel(espnow_device_channel,WIFI_SECOND_CHAN_NONE) );
      if(esp_now_init() != 0){
        if( ESPNOW_DEVICE_LOG_DEBUG ){
          Serial.println("[Error] initializing ESP-NOW");
        }
        return false;
      }
      
      if( ESPNOW_DEVICE_LOG_DEBUG ){
        Serial.println("ESPNNOW init!!");
      }

      connection_led = false;
      connection_led_change = true;

      #ifdef ESP8266
      esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      #endif

      // callback espnow
      esp_now_register_recv_cb(espnow_device_recive);

      // diagnostico
      if( ESPNOW_DEVICE_LOG_DEBUG ){
        Serial.printf(  "[ESPNOW] channel: %d\t%d\n", espnow_device_get_channel() );
        Serial.print(   "[ESPNOW] MAC:  ");Serial.println(WiFi.macAddress());
        Serial.println( "[ESPNOW Device] begin!" );
      }

      if( ESPNOW_DEVICE_SERVER ){
        // notifier
        notifier.connect_client( ESPNOW_DEVICE_NAME, espnow_device_broadcast_mac );
        notifier.delay_ms_notify = delay_ms_notify;
        notifier.waiting_ms_disconnect = waiting_ms_disconnect;
      }

      espnow_device_init = true;

      return true;
    }

    // deinit
    void deinit(){
      espnow_device_init = false;
      esp_now_deinit();
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
    }

    //----------------------------------------------------------------------------------------
    // begin
    //----------------------------------------------------------------------------------------

    void begin_server( const char * name = "SERVER", const char * password = "banana" ){
      ESPNOW_DEVICE_SERVER = true;
      strncpy( ESPNOW_DEVICE_NAME, name, ESPNOW_DEVICE_NAME_SIZE );
      ESPNOW_DEVICE_PASSWORD = password;
      init();
    }

    void begin_client( const char * name = "CLIENT" ){
      ESPNOW_DEVICE_SERVER = false;
      strncpy( ESPNOW_DEVICE_NAME, name, ESPNOW_DEVICE_NAME_SIZE );
      init();
    }


    //----------------------------------------------------------------------------------------
    // connect
    //----------------------------------------------------------------------------------------

    bool connect( const char * _name, const char *_password, const uint8_t *mac ){
      if( ESPNOW_DEVICE_SERVER ) return false;
      if( connections_counter >= connections_counter_max ) return false;
      connections[connections_counter].connect_server( _name, _password );
      connections[connections_counter].set_mac( mac );
      connections[connections_counter].delay_ms_notify = delay_ms_notify;
      connections[connections_counter].waiting_ms_disconnect = waiting_ms_disconnect;
      connections_counter++;
      return true;
    }

    bool connect( const char * _name, const char *_password = "" ){
      if( ESPNOW_DEVICE_SERVER ) return false;
      if( connections_counter >= connections_counter_max ) return false;
      connections[connections_counter].connect_server( _name, _password );
      connections[connections_counter].delay_ms_notify = delay_ms_notify;
      connections[connections_counter].waiting_ms_disconnect = waiting_ms_disconnect;
      connections_counter++;
      return true;
    }

    //----------------------------------------------------------------------------------------
    // update
    //----------------------------------------------------------------------------------------

    void update(){

      if( !espnow_device_init ) return;

      bool led_act = false;

      uint8_t n_connections = connections_counter;

      // se for um client single server
      if( !ESPNOW_DEVICE_SERVER && single_server ){
        if( connections[0].connected() ) n_connections = 1;
      }

      // update all connections
      for(int i=0;i<n_connections;i++){

        bool notify = connections[ i ].update_flags();
        
        if( !connections[ i ].Connected ){ // perdeu connexão
          
          if( connections[ i ].fall() ){
            call( ESPNOW_EVT_DISCONNECTED, i );
          }
          
          if( ESPNOW_DEVICE_SERVER ){
            // deleta a conexão e realoca as outras
            for(int j=i+1;j<connections_counter;j++)
              connections[j-1] = connections[j];
            i--; // o atual é deletado necessitando reiniciar a leitura
            connections_counter--;
            n_connections--;
          }else{
            connections[ i ].searching_mac = true;
          }
        }else{
          led_act = true;
          if( notify ){
            call( ESPNOW_EVT_NOTIFY, i );
            connections[ i ].send();
          }
        }
      }

      // public notification
      if( ESPNOW_DEVICE_SERVER ){
        if( connections_counter < connections_counter_max ){
          if( notifier.update_flags(true) ){
            call( ESPNOW_EVT_PUBLIC_NOTIFY, -1 );
            notifier.send();
          }
        }
      }

      // led update
      connection_led_change = ( led_act != connection_led );
      if( connection_led_change ){
        connection_led = led_act;
        if( connection_led_pin >= 0 ) digitalWrite(connection_led_pin, connection_led == connection_led_state_on );
      }

    }

    // ------------------------------------------------------------------------------------
    // callback
    // ------------------------------------------------------------------------------------
    void recive(const uint8_t * mac,const uint8_t *data, int len){

      if( !espnow_device_init ) return;
      
      if( ESPNOW_DEVICE_LOG_DEBUG ){
        Serial.printf( "\n\n-> recive: %s [%d]\n", mac2str(mac).c_str(), len );
      }

      #ifdef ESP8266
        espnow_device_frame_t _pack;
        espnow_device_frame_t *pack = &_pack;
        memcpy( (uint8_t*)pack, data, len );
      #else
        espnow_device_frame_t *pack = (espnow_device_frame_t*) data;
      #endif

      if( ESPNOW_DEVICE_LOG_DEBUG ){
        Serial.printf( "[name: %s][code: %d ][connection counter: %d ]\n", pack->name, pack->code, connections_counter );
      }
      
      if( pack->code != ESPNOW_DEVICE_CODE ) return;

      uint8_t n_connections = connections_counter;

      // se for um client single server
      if( !ESPNOW_DEVICE_SERVER && single_server ){
        if( connections[0].connected() ) n_connections = 1;
      }


      // check all connections
      for(int i=0;i<n_connections;i++){

        if( strcmp( connections[i].remote_name, pack->name ) == 0 ){
          
          // log
          if( ESPNOW_DEVICE_LOG_DEBUG ){
            Serial.printf( "[*%d*]\n",i);
          }
        
          // Local Client
          if( !ESPNOW_DEVICE_SERVER ){
            
            // se estiver buscando um dispositivo
            if( connections[ i ].searching_mac ){
              
              //log
              if( ESPNOW_DEVICE_LOG_DEBUG ){
                Serial.printf( "[found server MAC]\n", i );
              }
              
              // 
              connections[ i ].set_mac( mac );
              if( single_server ){
                ESPNOW_device_connection cnn_temp = connections[ 0 ];
                connections[ 0 ] = connections[ i ];
                connections[ i ] = cnn_temp;
                i = 0;
                n_connections = 1;
              }

            }
          }
          
          if( memcmp( connections[ i ].remote_mac, mac, 6 ) != 0 ) return;
          
          if( ESPNOW_DEVICE_LOG_DEBUG ){
            Serial.printf( "[name: %s connection: %d]\n", connections[ i ].remote_name, i );
          }

          connections[ i ].update_recive();
          connections[ i ].frame_in = *pack;

          if( connections[ i ].rise() ) call( ESPNOW_EVT_CONNECTED, i );
          call( ESPNOW_EVT_RECIVE, i );

          return;

        }

      }

      if( ESPNOW_DEVICE_SERVER && connections_counter<connections_counter_max ){
        // [ decode aqui ]
        if( strncmp(pack->name_rx, ESPNOW_DEVICE_NAME, ESPNOW_DEVICE_NAME_SIZE ) == 0 ){
          
          connections[ connections_counter ].connect_client( pack->name, mac);
          connections[ connections_counter ].frame_in = *pack;
          connections[ connections_counter ].delay_ms_notify = delay_ms_notify;
          connections[ connections_counter ].waiting_ms_disconnect = waiting_ms_disconnect;

          if( ESPNOW_DEVICE_LOG_DEBUG ){
            Serial.printf( "[name: %s connection: %d]\n", connections[ connections_counter ].remote_name, connections_counter );
          }
          connections_counter++;

          if( connections[ connections_counter-1 ].rise() ) call( ESPNOW_EVT_CONNECTED, connections_counter-1 );
          call( ESPNOW_EVT_RECIVE, connections_counter-1 );
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

#endif