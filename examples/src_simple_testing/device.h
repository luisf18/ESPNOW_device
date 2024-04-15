#ifndef ESPNOW_DEVICE_H
#define ESPNOW_DEVICE_H

#include "ESPNOWSerial.h"

/*/
#include "ESPNOW_device_defines.h"
#define ESPNOW_DEVICE_PUBLIC_ID 38223

template < typename T >
T buffer_get(uint8_t *p){
  T val;
  memcpy( (uint8_t*)&val, p, sizeof(T) );
  return val;
}

template < typename T >
uint32_t buffer_save(uint8_t *p, T val){
  memcpy( p, (uint8_t*)&val, sizeof(T) );
  return sizeof(T);
}
/*/


// protótipo da função de callback
void recive_radio( const uint8_t* mac, const uint8_t* data, int len);


class ESPNOW_device_cnn{

  #ifdef ESP32
  esp_now_peer_info_t peerInfo;
  #endif
  
  // temporização
  uint32_t Timeout_disconnect = 0;
  uint32_t Timeout_offline    = 0;
  uint32_t Timeout_scan       = 0;
  uint32_t Timeout_write      = 0;
  
  // variaveis de estado
  bool  Online    = false;
  bool  Change    = false;
  bool  Connected = false;

  // protocol
  uint8_t  load_size = 0;

  // Modos de operação
  enum{
    OFF = 0,
    SCAN,
    CONNECTING,
    CONNECTED,
    FREE
  };
  uint8_t mode = OFF;

  void set_mode( uint8_t _mode ){
    if( _mode > CONNECTED ) return;
    mode = _mode;
  }

  void find_device(){
    
  }

  public:

    // temporização valores de intervalos e delays
    //boolean  Send_flag = false;
    uint32_t waiting_ms_offline    = 500;
    uint32_t waiting_ms_disconnect = 300;
    uint32_t waiting_ms_write      = 0;
    uint32_t delay_ms_write        = 50;

    // services
    enum{
      SERVICE_GENERIC = 0,
      SERVICE_TEXT,
      SERVICE_CHANNELS
    };

    //----------------------------------------------------------------------------------------
    // Dados dos dispositivos
    //----------------------------------------------------------------------------------------
    typedef struct{
      char     name[15]    = "ESPNOW_DEVICE"; // Nome         [ 15 Bytes ]
      uint32_t type        = 0;               // tipo         [  2 Bytes ]
      uint8_t  mac[6];                        // endereço MAC [  6 Bytes ]
    }espnow_device_info;

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
      uint16_t id          = SERVICE_GENERIC; // identificador do serviço      [  2 Bytes ]
      uint8_t  len         = 0;               // tamanho da carga              [ *2 Bytes ]
      uint8_t  body[208];                     // identificador do serviço      [ 0-208 Bytes ]
    }espnow_device_frame;
  
  //----------------------------------------------------------------------------------------
  // Variaveis
  //----------------------------------------------------------------------------------------
  espnow_device_frame frame_in;
  espnow_device_frame frame_out;
  espnow_device_info  Local;
  espnow_device_info  Remote;
  String   password  = "banana";
  bool     is_server = false;

  //----------------------------------------------------------------------------------------
  // Funções de callback
  //----------------------------------------------------------------------------------------
  void (*callback_send  )(void) = nullptr;
  void (*callback_recive)(void) = nullptr;
  void set_send_callback(void (*f)()){ callback_send = f; }
  void set_recive_callback(void (*f)()){ callback_recive = f; }

  //----------------------------------------------------------------------------------------
  // Handler events
  //----------------------------------------------------------------------------------------
  void (*handle)(int) = nullptr;
  enum{
    EVT_SEND = 0,
    EVT_RECIVE,
    EVT_RISE_CONNECTION,
    EVT_FALL_CONNECTION,
    EVT_SCAN_NEW_DEVICE,
    EVT_SCAN_RESULT,
    EVT_LOST_CONNECTION
  };
  void set_handle_function(void (*f)(int)){ handle = f; }

  //----------------------------------------------------------------------------------------
  // Funções gerais
  //----------------------------------------------------------------------------------------
  //bool check_mac( uint8_t *Mac ){ return memcmp( Local.mac, Mac ) == 0; }
  
  String str(){
    return "Device " + String(Local.name) + ", type: " + String(Local.type) + ", mac: " + ESPNOWSerial.mac2str(Local.mac);
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
  bool begin( const char *_name, bool _is_server, uint32_t _type = 0, const char *_password = "" ){ //, uint8_t *Mac ){
    strncpy( Local.name, _name, 15 );
    Local.type = _type;
    password   = _password;
    is_server  = _is_server;
    //memcpy( Local.mac, Mac, 15 );

    // Inicia o ESPNOW enviando e recebendo em modo broadcast
    //ESPNOWSerial.begin( mac );
    ESPNOWSerial.begin();
    ESPNOWSerial.setTimeout(20);
    ESPNOWSerial.setWriteDelay(10);
    ESPNOWSerial.canReciveFrom_anyDevice();

    // set callback
    ESPNOWSerial.setReciveCallback( recive_radio );
      
    // inicia no modo scan
    mode = SCAN;
    
    // intervalo de escrita
    waiting_ms_write = ( is_server ? 500 : 50 );
    
    Serial.printf( ">> begin %s!\n", (is_server?"server":"client"));
    return true;
  }

  //----------------------------------------------------------------------------------------
  // Funções basicas de leitura e escrita
  //----------------------------------------------------------------------------------------

  // states
  boolean fall(){ return Change&&(!Online); }
  boolean rise(){ return Change&&Online; }
  boolean change(){ return Change; }
  boolean online(){ return Online; }
  boolean connected(){ return mode == CONNECTED; }

  // update
  void update(){
    
    //if( !EN ) return;

    uint32_t time = millis();

    boolean Online_act  = time < Timeout_offline;
    boolean Connect_act = time < Timeout_disconnect;
    
    // scan
    if( mode == SCAN ){
      if( time >= Timeout_scan ){
        if(handle != nullptr) handle( EVT_SCAN_RESULT );
        //scan( scan_duration );
      }
    }

    // send
    if( (delay_ms_write>0) && (mode==CONNECTING||mode==CONNECTED) ){
      if( time >= Timeout_write ){
        Timeout_write = time + delay_ms_write;
        frame_out.len = 0;
        if(callback_send != nullptr) callback_send();
        if(handle != nullptr) handle( EVT_SEND );
        send();
      }
    }

    if( Online_act == Online ){
      Change = false;
    }else{
      Online = Online_act;
      Change = true;
      if(handle != nullptr) handle( Online ? EVT_RISE_CONNECTION : EVT_FALL_CONNECTION );
    }

    if( !Connect_act && mode==CONNECTED ){
      mode = CONNECTING;
      Connected = false;
      if(handle != nullptr) handle( EVT_LOST_CONNECTION );
    }

  }


  //----------------------------------------------------------------------------------------
  // Scan
  //----------------------------------------------------------------------------------------
  /*/
  void scan( uint32_t duration ){
    scan_duration = duration;
    Timeout_scan = millis() + scan_duration;
    scan_devices_count = 0;
    mode = SCAN;
  }
  /*/

  //----------------------------------------------------------------------------------------
  // Connecting
  //----------------------------------------------------------------------------------------
  // client connect to server
  void connect( const char * _name, uint8_t *mac ){ // passando um mac força uma conexão com este mac
    if( is_server ) return;
    connect( _name );
    ESPNOWSerial.peer( mac );
    mode = CONNECTED;
    Connected = true;
  }
  
  // sem mac o cliente busca um server que possua este nome
  void connect( const char * _name ){
    if( is_server ) return;
    mode = CONNECTING;
    Connected = false;
    strcpy( Remote.name, _name );
    strcpy( frame_out.name_rx, _name );
  }

  //----------------------------------------------------------------------------------------
  // Recive
  //----------------------------------------------------------------------------------------
  void recive( const uint8_t *MAC, const uint8_t *p_pack, const uint8_t len ){
    //if( !EN ) return;
    
    // int load_len = len - (sizeof(espnow_device_frame)-sizeof(espnow_device_protocol::load));
    // if( load_len >= 0 && load_len <= sizeof(espnow_device_frame::body) ) return;

    // copia os bytes como um 
    espnow_device_frame *pack = (espnow_device_frame*) p_pack;

    if( mode == SCAN ){
        //Serial.println("[SCAN] add device...\n\n");
        //if( scan_add_device(&pack,MAC) ){
        //  Serial.printf( "SCAN[%i] ", scan_devices_count-1 );
        //  Serial.println( scan_devices[scan_devices_count-1].toString() );
        //  if(handle != nullptr) handle(EVT_SCAN_NEW_DEVICE);
        //}
      }else if( mode == CONNECTING ){
        
        Serial.println("[CONNECTING]");

        // se houver descpriptografia entra aqui...
        // decode( &pack );

        // verifica o nome do dispositivo
        if( is_server ){ // o server aguarda algum dispositivo "acertar" seu nome
          if( strcmp(pack->name_rx,Local.name) != 0 ) return; // verifica o nome do dispositivo destino
        }else{ // o client aguarda receber um pacote com do server para descobrir seu MAC
          if( strcmp(pack->name,Remote.name)    != 0 ) return; // verifica o nome do dispositivo origem
        }

        // ----------- validação completa! ------------- //
        
        // realiza conexão
        Serial.printf("[salva o codigo mac do %s][modo = conectado]\n\n", (is_server?"client":"server"));

        // temporização
        Timeout_offline = millis() + waiting_ms_offline;
        Timeout_disconnect = Timeout_offline + waiting_ms_disconnect;
        
        // copia os dados
        frame_in = *pack;
        
        // armazena os dados do dispositivo remoto
        memcpy(Remote.mac,MAC,6);
        strcpy(Remote.name,pack->name);
        Remote.type = pack->type;
        
        // atualiza o estado
        mode = CONNECTED;
        Connected = true;

      }else if( mode == CONNECTED ){
        
        Serial.println("[CONNECTED][Recive LOAD]");
        
        // verifica o mac
        if( memcmp( Remote.mac, MAC, 6      ) != 0 ) return;
        // verifica o nome do dispositivo
        if( strcmp( Remote.name, pack->name ) != 0 ) return;
        
        //decode( &pack );

        if( is_server ){
          if( strcmp(pack->name_rx,Local.name) != 0 ) return;
        }

        // ----------- validação completa! ------------- //
        
        // temporização
        Timeout_offline = millis() + waiting_ms_offline;
        Timeout_disconnect = Timeout_offline + waiting_ms_disconnect;
        
        // ação
        frame_in = *pack;
        if(callback_recive != nullptr) callback_recive();
        if(handle          != nullptr) handle(EVT_RECIVE);
      }

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

};

ESPNOW_device_cnn ESPNOW_device;

void recive_radio( const uint8_t* mac, const uint8_t* data, int len){
  ESPNOW_device.recive(mac,data,len);
}


#endif