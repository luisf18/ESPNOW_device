#ifndef ESPNOW_DEVICE_CONNECTION_H
#define ESPNOW_DEVICE_CONNECTION_H

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
// constantes
//========================================================================================
#define espnow_device_connection_max 10
const uint8_t espnow_device_broadcast_mac[6]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


//========================================================================================
// callback functions
//========================================================================================
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *incomingData, int len);
void espnow_device_sent(const uint8_t *mac_addr, esp_now_send_status_t status);
#elif defined(ESP8266)
void espnow_device_recive(uint8_t * mac, uint8_t *incomingData, uint8_t len);
void espnow_device_sent(uint8_t *mac_addr, uint8_t sendStatus);
#endif


//========================================================================================
// basic functions
//========================================================================================

String mac2str(const uint8_t *mac ){
  char char_str[18];
  snprintf(char_str, sizeof(char_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  String ret = char_str;
  return ret;
}

void espnow_set_mac( uint8_t *new_mac ){
  #if ESP32
      //esp_wifi_set_max_tx_power(84);
      //ESP32 Board add-on before version < 1.0.5
      //esp_wifi_set_mac(ESP_IF_WIFI_STA, &newMACAddress[0]);
      // ESP32 Board add-on after version > 1.0.5
      esp_wifi_set_mac(WIFI_IF_STA, new_mac);
  #else
      // For Soft Access Point (AP) Mode
      //wifi_set_macaddr(SOFTAP_IF, &newMACAddress[0]);
      // For Station Mode
      wifi_set_macaddr(STATION_IF, new_mac);
  #endif
}


//========================================================================================
// Enum
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

// services
enum espnow_device_service_t{
  SERVICE_GENERIC = 0,
  SERVICE_TEXT,
  SERVICE_CHANNELS
};


//========================================================================================
// Estruturas
//========================================================================================

typedef struct{
  char     name[15]    = "ESPNOW_DEVICE"; // Nome         [ 15 Bytes ]
  uint32_t type        = 0;               // tipo         [  2 Bytes ]
  uint8_t  mac[6];                        // endereço MAC [  6 Bytes ]
  String   password    = "banana";        // senha
}espnow_device_info_t;

typedef struct{
  // HEADER [ 19 Bytes ] ---------------------------------------------------------------
  char     name[15]    = "ESPNOW_DEVICE"; // Nome do dispositivo origem    [ 15 Bytes ]
  uint32_t type        = 0;               // tipo de dispositivo origem    [  2 Bytes ]
  // HEADER LOCK [ 19 Bytes ] ----------------------------------------------------------
  uint16_t random      = 0;               // Dados de segurança da conexão [  2 Bytes ]
  char     name_rx[15] = "RX";            // Dispositivo destino           [ 15 Bytes ]
  // Body (carga util) [ 212 Bytes ] ---------------------------------------------------
  uint16_t service     = SERVICE_GENERIC; // identificador do serviço      [  2 Bytes ]
  uint8_t  len         = 0;               // tamanho da carga              [ *2 Bytes ]
  uint8_t  body[208];                     // identificador do serviço      [ 0-208 Bytes ]
}espnow_device_frame_t;


//========================================================================================
// Variaveis
//========================================================================================
uint8_t espnow_device_channel = 1;
espnow_device_info_t espnow_device_local_info;



//========================================================================================
// Peer
//========================================================================================
#ifdef ESP32
bool peer( const uint8_t *mac, esp_now_peer_info_t *peerInfo ){
  // Register peer device
  memcpy(peerInfo->peer_addr, mac, 6);
  peerInfo->channel = espnow_device_channel;
  peerInfo->encrypt = false;
  // Add peer
  if (esp_now_add_peer(peerInfo) != ESP_OK){ Serial.println("Failed to add peer"); return false; }
  return true;
}
#elif defined(ESP8266)
bool peer( const uint8_t *mac ){
  esp_now_add_peer( mac, ESP_NOW_ROLE_SLAVE, espnow_device_channel, NULL, 0);
  return true;
}
#endif



//========================================================================================
// Connection
//========================================================================================
class ESPNOW_device_connection{
  
  #ifdef ESP32
  esp_now_peer_info_t peerInfo;
  #endif

  // dados do dispositivo remoto
  //char     name[15] = "ESPNOW_DEVICE";                         // Nome         [ 15 Bytes ]
  //uint32_t type     = 0;                                       // tipo         [  2 Bytes ]
  //uint8_t  mac[6]   = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // endereço MAC [  6 Bytes ]
  //String   password = "banana";                                // senha

  // temporização
  uint32_t Timeout_disconnect = 0;
  uint32_t Timeout_notify     = 0;

  // variaveis de estado
  bool  Connected = false;
  bool  Change    = false;

  public:

  //----------------------------------------------------------------------------------------
  // Variaveis
  //----------------------------------------------------------------------------------------

  // temporização valores de intervalos e delays
  uint32_t waiting_ms_disconnect = 5000;
  uint32_t delay_ms_notify       = 300;

  // conexão
  espnow_device_frame_t frame_in, frame_out;
  espnow_device_info_t  Remote;
  bool remote_server = false;
  bool searching_mac = false;

  //----------------------------------------------------------------------------------------
  // State Flags
  //----------------------------------------------------------------------------------------
  boolean fall(){ return Change&&(!Connected); }
  boolean rise(){ return Change&&Connected; }
  boolean change(){ return Change; }
  boolean connected(){ return Connected; }

  //----------------------------------------------------------------------------------------
  // Begin
  //----------------------------------------------------------------------------------------

  void set_mac( const uint8_t *mac ){

    bool is_broadcast = ( memcmp( Remote.mac, espnow_device_broadcast_mac, 6 ) == 0 );

    // Remote device info
    memcpy( Remote.mac, mac, 6 );

    #ifdef ESP32
      peer( Remote.mac, &peerInfo );
    #elif defined(ESP8266)
      peer( Remote.mac );
    #endif

    if( remote_server && !is_broadcast ){
      Change = true;
      Connected = true;
      searching_mac = false;
      Serial.printf( ">> connect to remote server %s!\n", Remote.name);
    }

  }

  // deve aguardar a notificação publica do server para poder se conectar
  void connect_to_remote_server( const char *name, const char *_password ){

    remote_server = true;
    searching_mac = true;

    // Remote device info
    strncpy( Remote.name, name, 15 );
    Remote.password = _password;
    
    // frame header
    strncpy( frame_out.name, espnow_device_local_info.name, 15 );
    frame_out.type = espnow_device_local_info.type;
    strncpy( frame_out.name_rx, name, 15 );

    // flags
    Change = false;
    Connected = false;
    
    set_mac( espnow_device_broadcast_mac );

    Serial.printf( ">> request connection to server %s!\n", name);

  }

  // um dispositivo client se connectou ao dispositivo local
  void connect_to_remote_client( const char *name, const uint8_t *mac ){

    remote_server = false;
    searching_mac = false;

    // Remote device info
    strncpy( Remote.name, name, 15 );
    
    // frame header
    strncpy( frame_out.name, espnow_device_local_info.name, 15 );
    frame_out.type = espnow_device_local_info.type;
    strncpy( frame_out.name_rx, name, 15 );
    
    // flags
    Change = true;
    Connected = true;

    // peer
    set_mac( mac );

    Serial.printf( ">> connect to remote client %s!\n", name);

  }
    

  //----------------------------------------------------------------------------------------
  // Send
  //----------------------------------------------------------------------------------------
  // armazena em frame_out.body os bytes de load depois envia frame_out
  void send( uint8_t *load, uint8_t load_len ){
    frame_out.len = constrain(load_len,0,208);
    memcpy( (uint8_t*)frame_out.body, load, frame_out.len );
    send();
  }

  // envia frame_out
  void send(){
    esp_now_send( Remote.mac, (uint8_t*)&frame_out, sizeof(frame_out) - (208-frame_out.len) );
  }

  //----------------------------------------------------------------------------------------
  // update
  //----------------------------------------------------------------------------------------
  void update(){
    boolean Connect_act = millis() < Timeout_disconnect;
    if( Connect_act == Connected ){
      Change    = false;
    }else{
      Connected = Connect_act;
      Change    = true;
    }
  }

  bool update_notify( bool _send = true ){
    uint32_t time = millis();
    if( time >= Timeout_notify ){
      Timeout_notify = time + delay_ms_notify;
      if(_send) send();
      return true;
    }
    return false;
  }

  void update_recive(){
    uint32_t time = millis();
    Timeout_disconnect = time + waiting_ms_disconnect;
    Change = !Connected;
    Connected = true;
  }

};


//========================================================================================
// ESPNOW_device
//========================================================================================
class ESPNOW_device{

  #ifdef ESP32
  esp_now_peer_info_t peerInfo;
  #endif

  bool is_server = true;

  // notifier
  uint32_t Timeout_notify = 0;

  public:
  
  //----------------------------------------------------------------------------------------
  // Variaveis
  //----------------------------------------------------------------------------------------

  // connexões
  bool    always_public_notify = false;
  uint8_t connections_counter_max = 0;
  uint8_t connections_counter = 0;
  ESPNOW_device_connection connections[espnow_device_connection_max];
  uint32_t delay_ms_notify = 100;
  espnow_device_frame_t frame_notify;

  //----------------------------------------------------------------------------------------
  // Begin e deinit
  //----------------------------------------------------------------------------------------

  // deinit
  void deinit(){
    esp_now_deinit();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  }

  bool init(){

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    ESP_ERROR_CHECK( esp_wifi_set_channel(espnow_device_channel,WIFI_SECOND_CHAN_NONE) );
    if(esp_now_init() != 0){
      Serial.println("[Error] initializing ESP-NOW");
      return false;
    }
    Serial.println("\n\nBEGIN ESPNOW!!");

    #ifndef ESP32
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    #endif

    // Register peer device
    #ifdef ESP32
      memcpy(peerInfo.peer_addr, espnow_device_broadcast_mac, 6);
      peerInfo.channel = espnow_device_channel;
      peerInfo.encrypt = false;
      // Add peer
      if (esp_now_add_peer(&peerInfo) != ESP_OK){ Serial.println("Failed to add peer"); }
    #else
      esp_now_add_peer(espnow_device_broadcast_mac, ESP_NOW_ROLE_SLAVE, espnow_device_channel, NULL, 0);
    #endif

    // callback espnow
    esp_now_register_recv_cb(espnow_device_recive);
    //esp_now_register_send_cb(espnow_device_sent  );

    // diagnostico
    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);
    espnow_device_channel = primary;
    Serial.printf(  "[ESPNOW] channel: %d\t%d\n",primary,second);
    Serial.print(   "[ESPNOW] MAC:  ");Serial.println(WiFi.macAddress());
    Serial.println( "[ESPNOW Device] begin!" );
    
    connections_counter = 0;

    return true;
  }
  
  void beginServer( const char *Name ){
    begin( Name, true );
  }
    
  void beginClient( const char *Name ){
    begin( Name, false );
  }
  
  //virtual 
  bool begin( const char *_name, uint32_t _is_server = true, uint32_t _type = 0, const char *_password = "", uint8_t channel = 1, uint8_t _connections_counter_max = 1 ){

    // copia os dados do dispositivo local
    strncpy( espnow_device_local_info.name, _name, 15 );
    espnow_device_local_info.type     = _type;
    espnow_device_local_info.password = _password;
    esp_read_mac( espnow_device_local_info.mac, ESP_MAC_WIFI_STA);
    espnow_device_channel = channel;
    
    connections_counter_max = constrain( _connections_counter_max, 0, espnow_device_connection_max );

    // init frame_notify
    strncpy( frame_notify.name, espnow_device_local_info.name, 15 );
    frame_notify.type = espnow_device_local_info.type;
    strncpy( frame_notify.name_rx, "", 15 );

    return init();
  }

  //----------------------------------------------------------------------------------------
  // Funções gerais
  //----------------------------------------------------------------------------------------
  
  String str(){
    return "Device " + String(espnow_device_local_info.name) + ", type: " + String(espnow_device_local_info.type) + ", mac: " + mac2str(espnow_device_local_info.mac);
  }

  int find_mac( const uint8_t *mac ){
    for(int i=0;i<connections_counter;i++){ if( memcmp(connections[i].Remote.mac, mac, 6) == 0 ) return i; }
    return -1;
  }

  int find_name( const char *name ){
    for(int i=0;i<connections_counter;i++){ if( strncmp(connections[i].Remote.name, name, 15) == 0 ) return i; }
    return -1;
  }

  //----------------------------------------------------------------------------------------
  // Update
  //----------------------------------------------------------------------------------------

  // update
  void update(){
    
    if( always_public_notify || is_server ){
      if( always_public_notify || connections_counter < connections_counter_max ){
        if( millis() > Timeout_notify ){
          Timeout_notify = millis() + delay_ms_notify;
          call( ESPNOW_EVT_PUBLIC_NOTIFY, -1 );
          esp_now_send( espnow_device_broadcast_mac, (uint8_t*)&frame_notify, 250 );
        }
      }
    }
    
    // update all connections
    for(int i=0;i<connections_counter;i++){
      connections[i].update();
      if( connections[i].connected() ){
        Serial.printf( "connection %i\n", i );
        if( connections[i].rise() ) call( ESPNOW_EVT_CONNECTED, i );
        if( connections[i].update_notify() ){
          call( ESPNOW_EVT_NOTIFY, i );
          connections[i].send();
        }
      }else{
        if( connections[i].fall() ) call( ESPNOW_EVT_DISCONNECTED, i );
      }
    }
  }

  //----------------------------------------------------------------------------------------
  // Connect
  //
  // o dispositivo local inicia uma conexão como client de um dispositivo de nome "_name"
  // sem o mac: descobre pelo nome via pacotes abertos de notify do server
  //----------------------------------------------------------------------------------------

  bool connect( const char * _name, const char *_password, const uint8_t *mac ){
    if( connections_counter >= connections_counter_max ) return false;
    connections[connections_counter].connect_to_remote_server( _name, _password );
    connections[connections_counter].set_mac( mac );
    connections_counter++;
    return true;
  }
  
  bool connect( const char * _name, const char *_password = "" ){
    if( connections_counter >= connections_counter_max ) return false;
    connections[connections_counter].connect_to_remote_server( _name, _password );
    connections_counter++;
    return true;
  }

  //----------------------------------------------------------------------------------------
  // Handler events
  //----------------------------------------------------------------------------------------
  void (*handle)(espnow_device_event_t,int) = nullptr;
  void set_handle_function(void (*f)(espnow_device_event_t,int)){ handle = f; }
  void call( espnow_device_event_t EVT, int id ){ if( handle != nullptr ) handle(EVT,id);  }

  //----------------------------------------------------------------------------------------
  // Recive
  //----------------------------------------------------------------------------------------
  void recive( const uint8_t *MAC, const uint8_t *p_pack, const uint8_t len ){

    Serial.printf( "recive: %s [%d]\n", mac2str(MAC).c_str(), len );

    // copia os bytes como um 
    espnow_device_frame_t *pack = (espnow_device_frame_t*) p_pack;

    int connection_id = find_name( pack->name );

    if( connection_id == -1 ){ // conexão não encontrada
      if( connections_counter < connections_counter_max ){
        // decode(pack); // decodifica o pacote...
        // verificação do nome destino da requisição
        if( strcmp(pack->name_rx,espnow_device_local_info.name) != 0 ) return;

        // --------------- validação completa / nova conexão como server --------------------

        connections[ connections_counter ].connect_to_remote_client( pack->name, MAC );
        connections_counter++;
        call( ESPNOW_EVT_CONNECTED, connections_counter-1 );

      }
    }else{ // conexão encontrada

      // [server] o dispositivo local recebe a connexão
      if( is_server ){
        
        // 1 - verifica o codigo MAC
        if( memcmp(connections[ connection_id ].Remote.mac, MAC, 6 ) != 0 ) return;
        
        // 2 - decodifica e verifica o nome de destino
        // decode( &pack ); // se houver descpriptografia entra aqui...
        if( strcmp(connections[ connection_id ].Remote.name, pack->name_rx) != 0 ) return;

        // -------------------------- validação completa -----------------------------------

        connections[ connection_id ].update_recive();
        
        // 3 - caso não esteja conectado
        if( connections[ connection_id ].rise() ) call( ESPNOW_EVT_CONNECTED, connection_id );

      }
      
      // [client] o dispositivo local inicia a connexão
      else{

        // -------------------------- validação completa -----------------------------------

        connections[ connection_id ].update_recive();
        
        // 1 - caso não esteja conectado
        if( connections[ connection_id ].rise() ){
          // salva o codigo mac e realiza o peer
          memcpy(connections[ connection_id ].Remote.mac,MAC,6);
          call( ESPNOW_EVT_CONNECTED, connection_id );
        }

      }

      connections[connection_id].frame_in = *pack;

    }

    call( ESPNOW_EVT_RECIVE, connection_id );

  }

};

ESPNOW_device ESPNOW_device;



// ===============================================================================
// External callback functions
// ===============================================================================
#ifdef ESP32
void espnow_device_recive(const uint8_t * mac,const uint8_t *data, int len){
    ESPNOW_device.recive(mac,data,len);
}
void espnow_device_sent(const uint8_t *mac, esp_now_send_status_t status) {
    //ESPNOW_device.recive(mac,data,len);
}
#elif defined(ESP8266)
// ....
#endif

#endif