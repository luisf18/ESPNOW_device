#ifndef ESPNOW_DEVICE_H
#define ESPNOW_DEVICE_H

// resolver problema de perda permanente de conexão
// reciclar as conexões perdidas...
// talves usar alocação dinamica

// ESPNOWSerial -> stand alone...

#include "ESPNOWSerial.h"

#define espnow_device_connection_max 10
const uint8_t espnow_device_broadcast_mac[6]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// protótipo da função de callback
void recive_radio( const uint8_t* mac, const uint8_t* data, int len);


//----------------------------------------------------------------------------------------
// Dados dos dispositivos
//----------------------------------------------------------------------------------------
typedef struct{
  char     name[15]    = "ESPNOW_DEVICE"; // Nome         [ 15 Bytes ]
  uint32_t type        = 0;               // tipo         [  2 Bytes ]
  uint8_t  mac[6];                        // endereço MAC [  6 Bytes ]
}espnow_device_info;

// Local Info
espnow_device_info espnow_device_local_info;

// services
enum espnow_device_service_t{
  SERVICE_GENERIC = 0,
  SERVICE_TEXT,
  SERVICE_CHANNELS
};

//----------------------------------------------------------------------------------------
// Bytes enviados via espnow
//----------------------------------------------------------------------------------------
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
}espnow_device_frame;


//----------------------------------------------------------------------------------------
// Events (for handle functions)
//----------------------------------------------------------------------------------------
enum espnow_device_event_t{
  EVT_NULL = 0,           // nenhuma ação
  EVT_PUBLIC_NOTIFY,      // [update  ] Notify
  EVT_NOTIFY,             // [update  ] Notify
  EVT_SEND,               // [update ou asincrono] chamada assincrona ou timer write
  EVT_RECIVE,             // [callback] recive
  EVT_CONNECTED,          // [callback] recive
  EVT_DISCONNECTED,       // [update  ] update
  EVT_CONNECTION_REQUEST  // [callback] recive
};

//----------------------------------------------------------------------------------------
// Connection
//----------------------------------------------------------------------------------------
class ESPNOW_device_connection{
  
  #ifdef ESP32
  esp_now_peer_info_t peerInfo;
  #endif

  // temporização
  uint32_t Timeout_disconnect = 0;
  uint32_t Timeout_notify     = 0;

  // variaveis de estado
  bool  Connected = false;
  bool  Change    = false;

  public:

  // temporização valores de intervalos e delays
  uint32_t waiting_ms_disconnect = 500;
  uint32_t delay_ms_notify       = 50;

  //----------------------------------------------------------------------------------------
  // Variaveis
  //----------------------------------------------------------------------------------------
  espnow_device_frame frame_in;
  espnow_device_frame frame_out;
  espnow_device_info  Remote;
  String password      = "banana";
  bool   local_server  = false;
  

  // states
  boolean fall(){ return Change&&(!Connected); }
  boolean rise(){ return Change&&Connected; }
  boolean change(){ return Change; }
  boolean connected(){ return Connected; }

  //----------------------------------------------------------------------------------------
  // Begin
  //----------------------------------------------------------------------------------------
  // local server: recebe requisição de conexão
  // nome  -> do dispositivo remoto
  // senha -> não tem, é a do dispositivo remoto
  // mac   -> mac do dispositivo remoto
  void init_as_local_server( const char *_name, const uint8_t *Mac ){
    local_server = true;
    strncpy( Remote.name, _name, 15 );
    //delay_ms_write = ( is_server ? 500 : 50 );// intervalo de escrita
    Change = false;
    Connected = true;

    // frame header
    strncpy( frame_out.name, espnow_device_local_info.name, 15 );
    frame_out.type = espnow_device_local_info.type;
    strncpy( frame_out.name_rx, Remote.name, 15 );

    // [colocar Peer aqui]

    Serial.printf( ">> begin connection as server!\n");
  }

  // local client: local realiza a requisição de conexão
  // nome  -> do dispositivo remoto
  // senha -> do dispositivo remoto
  // mac   -> do dispositivo remoto (geralmente n tem... ele busca)
  void init_as_local_client( const char *_name, const char *_password = "" ){
    local_server = false;
    password = _password;
    strncpy( Remote.name, _name, 15 );
    //delay_ms_write = ( is_server ? 500 : 50 );// intervalo de escrita
    Change = false;
    Connected = false;

    // frame header
    strncpy( frame_out.name, espnow_device_local_info.name, 15 );
    frame_out.type = espnow_device_local_info.type;
    strncpy( frame_out.name_rx, Remote.name, 15 );


    Serial.printf( ">> begin connection as client!\n");
  }

  // local client: local realiza a requisição de conexão
  // nome  -> do dispositivo remoto
  // senha -> do dispositivo remoto
  // mac   -> do dispositivo remoto (geralmente n tem... ele busca)
  void init_as_local_client( const char *_name, const char *_password, const uint8_t *MAC ){
    init_as_local_client( _name, _password );
    memcpy(Remote.mac,MAC,6);
    // [colocar Peer aqui]
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
    ESPNOWSerial.write( (uint8_t*)&frame_out, sizeof(frame_out) - (208-frame_out.len) );
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

  bool update_notify(){
    uint32_t time = millis();
    if( time < Timeout_notify ){
      Timeout_notify = time + delay_ms_notify;
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

class ESPNOW_device{
  
  //----------------------------------------------------------------------------------------
  // Modos de operação
  //----------------------------------------------------------------------------------------
  enum espnow_device_mode_t{
    OFF = 0,
    SCAN,
    CONNECTING,
    CONNECTED,
    FREE
  };

  espnow_device_mode_t mode = OFF;

  void set_mode( espnow_device_mode_t _mode ){
    mode = _mode;
  }

  public:
  
  //----------------------------------------------------------------------------------------
  // Funções gerais
  //----------------------------------------------------------------------------------------
  
  String str(){
    return "Device " + String(espnow_device_local_info.name) + ", type: " + String(espnow_device_local_info.type) + ", mac: " + ESPNOWSerial.mac2str(espnow_device_local_info.mac);
  }
  
  //----------------------------------------------------------------------------------------
  // Variaveis
  //----------------------------------------------------------------------------------------
  
  // local
  espnow_device_frame frame_in;
  String password      = "banana";

  // connexões
  bool enable_to_recive_connections = true;
  uint8_t connections_counter = 0;
  ESPNOW_device_connection connections[espnow_device_connection_max];

  int find_mac( const uint8_t *mac ){
    for(int i=0;i<connections_counter;i++){
      if( memcmp(connections[i].Remote.mac, mac, 6) == 0 ) return i;
    }
    return -1;
  }

  int find_name( const char *name ){
    for(int i=0;i<connections_counter;i++){
      if( strncmp(connections[i].Remote.name, name, 15) == 0 ) return i;
    }
    return -1;
  }

  //----------------------------------------------------------------------------------------
  // Begin
  //----------------------------------------------------------------------------------------
  
  void beginServer( const char *Name ){
    begin( Name, true );
  }
    
  void beginClient( const char *Name ){
    begin( Name, false );
  }
  
  //virtual 
  bool begin( const char *_name, uint32_t _type = 0, const char *_password = "" ){ //, uint8_t *Mac ){
    strncpy( espnow_device_local_info.name, _name, 15 );
    espnow_device_local_info.type = _type;
    password   = _password;
    //memcpy( espnow_device_local_info.mac, Mac, 15 );

    // Inicia o ESPNOW enviando e recebendo em modo broadcast
    //ESPNOWSerial.begin( mac );
    ESPNOWSerial.begin();
    ESPNOWSerial.setTimeout(20);
    ESPNOWSerial.setWriteDelay(10);
    ESPNOWSerial.canReciveFrom_anyDevice();

    // set callback
    ESPNOWSerial.setReciveCallback( recive_radio );
    
    Serial.println( ">> begin ESPNOW Device!" );
    return true;
  }

  //----------------------------------------------------------------------------------------
  // Update
  //----------------------------------------------------------------------------------------

  // update
  void update(){
    // update all connections
    for(int i=0;i<connections_counter;i++){
      connections[i].update();
      if( connections[i].connected() ){
        if( connections[i].rise()          ) call( EVT_CONNECTED, i );
        if( connections[i].update_notify() ) call( EVT_NOTIFY, i );
      }else{
        if( connections[i].fall()  ) call( EVT_DISCONNECTED, i );
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
    if( connections_counter >= espnow_device_connection_max ) return false;
    connections[connections_counter].init_as_local_client( _name, _password, mac );
    ESPNOWSerial.peer( (uint8_t*)mac );
    connections[connections_counter].update_recive();
    connections_counter++;
    return true;
  }
  
  
  bool connect( const char * _name, const char *_password = "" ){
    if( connections_counter >= espnow_device_connection_max ) return false;
    connections[connections_counter].init_as_local_client( _name, _password );
    connections_counter++;
    return true;
  }

  bool init_server_notify( uint32_t delay_ms_notify ){
    connect( "notify", "", espnow_device_broadcast_mac );
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
    // copia os bytes como um 
    espnow_device_frame *pack = (espnow_device_frame*) p_pack;

    int connection_id = find_name( pack->name );

    if( connection_id == -1 ){ // conexão não encontrada
      if( connections_counter < espnow_device_connection_max && enable_to_recive_connections ){
        // decode(pack); // decodifica o pacote...
        // verificação do nome destino da requisição
        if( strcmp(pack->name_rx,espnow_device_local_info.name) != 0 ) return;

        // --------------- validação completa / nova conexão como server --------------------

        connections[ connections_counter ].init_as_local_server( pack->name, MAC );
        connections_counter++;
        call( EVT_CONNECTED, connection_id );

      }
    }else{ // conexão encontrada

      // [server] o dispositivo local recebe a connexão
      if( connections[ connection_id ].local_server ){
        
        // 1 - verifica o codigo MAC
        if( memcmp(connections[ connection_id ].Remote.mac, MAC, 6 ) != 0 ) return;
        
        // 2 - decodifica e verifica o nome de destino
        // decode( &pack ); // se houver descpriptografia entra aqui...
        if( strcmp(connections[ connection_id ].Remote.name, pack->name_rx) != 0 ) return;

        // -------------------------- validação completa -----------------------------------

        connections[ connection_id ].update_recive();
        
        // 3 - caso não esteja conectado
        if( connections[ connection_id ].rise() ) call( EVT_CONNECTED, connection_id );

      }
      
      // [client] o dispositivo local inicia a connexão
      else{

        // -------------------------- validação completa -----------------------------------

        connections[ connection_id ].update_recive();
        
        // 1 - caso não esteja conectado
        if( connections[ connection_id ].rise() ){
          // salva o codigo mac e realiza o peer
          memcpy(connections[ connection_id ].Remote.mac,MAC,6);
          call( EVT_CONNECTED, connection_id );
        }

      }

    }

    call( EVT_RECIVE, connection_id );

  }

};

ESPNOW_device ESPNOW_device;

void recive_radio( const uint8_t* mac, const uint8_t* data, int len){
  ESPNOW_device.recive(mac,data,len);
}


#endif