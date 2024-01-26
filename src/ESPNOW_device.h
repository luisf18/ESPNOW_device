#ifndef ESPNOW_DEVICE_H
#define ESPNOW_DEVICE_H

#include "ESPNOWSerial.h"
#include "ESPNOW_device_defines.h"

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


void recive_radio( const uint8_t* mac, const uint8_t* data, int len);

class ESPNOW_device{

    uint32_t Timeout_disconnect = 0;
    uint32_t Timeout_offline    = 0;
  
  public:

    // radio package id
    enum{
      PUBLIC_INFO = 0,
      CLOSED_INFO
    };

    typedef struct{
      char     name[15]  = "TX";
      uint32_t color_rgb = 0xF00000;
      uint32_t utility   = 0; //ROBOT | DDR | SUMO | RAMP; // tipo de serviço que o dispositivo de origem esta prestando
      uint8_t  mac[6];
      String toString(){
        return "Device " + String(name) + ", utility: " + String(utility) + ", mac: " + ESPNOWSerial.mac2str(mac) + ", color: " + String(color_rgb,HEX);
      }
    }device_t;

    // SCAN
    uint32_t scan_duration;
    uint32_t scan_timeout;
    uint8_t  scan_devices_count = 0;
    device_t scan_devices[20];

    typedef struct{
      // OPEN [ 27 Bytes ] ---------------------------------------------------------------
      // Sobre o pacote [ 4 Bytes ]
      uint16_t public_id    = 38223;       // Public Radio code
      uint16_t type         = CLOSED_INFO; // 
      // Sobre o dispositivo de origen [ 23 Bytes ]
      char     name[15]     = "TX";
      uint32_t color_rgb    = 0xF00000;
      uint32_t utility      = 0; //ROBOT | DDR | SUMO | RAMP; // tipo de serviço que o dispositivo de origem esta prestando
      // LOCK [ 221 Bytes ] ---------------------------------------------------------------
      // Dados de segurança da conexão [ 4 Bytes ]
      uint16_t salt          = 0;
      uint16_t private_id    = 38223; // Private Radio code
      // Dispositivo destino [ 15 Bytes ]
      char     name_rx[15]   = "RX";
      // Dados (carga util) [ 202 Bytes]
      uint16_t service_count    = 0; // quantos serviços existem
      uint8_t  load[200];
    }espnow_device_protocol;

    template<uint8_t n>
    struct espnow_service{
      uint8_t  len = n+4;   // tamanho da carga
      uint16_t service = 0; // identificação do serviço
      uint8_t  load[n];
    };

    espnow_device_protocol data_in;
    espnow_device_protocol data_out;

    // dados do dispositivo linkado
    boolean  connected_flag = false;
    uint8_t  connected_mac[6];
    char     connected_name[15] = "Radio1";
    String   link_key      = "abacate"; // se for robô é um dado local

    // local device conf
    uint8_t  is_server = false;
    char     name[15] = "Ninja";
    uint8_t  mode   = OFF;
    boolean  Online = false;
    boolean  Change = false;
    //boolean  Send_flag = false;z
    uint32_t offline_timeout = 500;
    uint32_t disconnect_timeout = 300;
    uint32_t WriteDelay   = 50;
    uint32_t WriteTimeout = 0;
    
    void (*callback_send)(void) = nullptr;
    void (*callback_recive)(void) = nullptr;
    uint8_t load_size = 0;

    // Modos de operação
    enum{
      OFF = 0,
      SCAN,
      CONNECTING,
      CONNECTED,
      FREE
    };

    // handler events
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

    void set_mode( uint8_t _mode ){
      if( _mode > CONNECTED ) return;
      mode = _mode;
    }

    void set_send_callback(void (*f)()){ callback_send = f; }
    void set_recive_callback(void (*f)()){ callback_recive = f; }
    void set_handle_function(void (*f)(int)){ handle = f; }

    //// Begin /////////////////////////////////////////////////////////////////
    
    void beginServer( String _name ){
      begin( _name, true );
    }
    
    void beginClient( String _name ){
      begin( _name, false );
    }
    
    void begin( String _name, boolean _is_server ){
      is_server = _is_server;
      
      // copia o nome
      memcpy((uint8_t*)name,(uint8_t*)_name.c_str(),15);

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
      WriteDelay = ( is_server ? 500 : 50 );

      // Dados do dispositivo
      memcpy( data_out.name, name, sizeof(name) );
      
      Serial.printf( ">> begin %s!\n", (is_server?"server":"client"));
      Serial.printf( ">> sizeof espnow_device_protocol: %i bytes\n", sizeof(data_out));

    }

    ///// SCAN ///////////////////////////////////////////////////////////////////////////////////////
    void scan( uint32_t duration ){
      scan_duration = duration;
      scan_timeout = millis() + scan_duration;
      scan_devices_count = 0;
      mode = SCAN;
    }

    //// connection //////////////////////////////////////////////////////////////////////////////////
    // client connect to server
    void connect( String _name, uint8_t *mac ){ // passando um mac força uma conexão com este mac
      if( is_server ) return;
      connect( _name );
      ESPNOWSerial.peer( mac );
      mode = CONNECTED;
      connected_flag = true;
    }
    
    // sem mac o cliente busca um server que possua este nome
    void connect( String _name ){
      if( is_server ) return;
      mode = CONNECTING;
      connected_flag = false;
      strcpy( connected_name, _name.c_str() );
      strcpy( data_out.name_rx, _name.c_str() );
    }

    //// ENVIO E RECEBIMENTO /////////////////////////////////////////////////////////////////////////
    void recive( const uint8_t *MAC, const uint8_t *p_pack, const uint8_t len ){

      if( len < (sizeof(espnow_device_protocol)-200) && len > sizeof(espnow_device_protocol) ) return;

      espnow_device_protocol pack;
      memcpy( (uint8_t*) &pack, p_pack, len );

      if( pack.public_id != data_out.public_id ) return;
      
      if( mode == SCAN ){
        //Serial.println("[SCAN] add device...\n\n");
        if( scan_add_device(&pack,MAC) ){
          Serial.printf( "SCAN[%i] ", scan_devices_count-1 );
          Serial.println( scan_devices[scan_devices_count-1].toString() );
          if(handle != nullptr) handle(EVT_SCAN_NEW_DEVICE);
        }
      }else if( mode == CONNECTING ){
        
        Serial.println("[CONNECTING]");

        // se houver descpriptografia entra aqui...
        // decode( &pack );

        // verifica o nome do dispositivo
        if( is_server ){
          if( strcmp(pack.name_rx,name) != 0 ) return;
        }else{
          if( strcmp(pack.name,connected_name) != 0 ) return;
        }

        if( pack.private_id != data_out.private_id ) return;

        // ----------- validação completa! ------------- //
        // realiza conexão
        Serial.printf("[salva o codigo mac do %s][modo = conectado]\n\n", (is_server?"client":"server"));

        Timeout_offline = millis() + offline_timeout;
        Timeout_disconnect = Timeout_offline + disconnect_timeout;
        data_in = pack;
        memcpy(connected_mac,MAC,6);
        strcpy(connected_name,pack.name);
        mode = CONNECTED;
        connected_flag = true;

      }else if( mode == CONNECTED ){
        Serial.println("[CONNECTED][Recive LOAD]");
        
        // verifica o mac
        if( memcmp( connected_mac, MAC, 6 ) != 0 ) return;
        // verifica o nome do dispositivo
        if( strcmp(pack.name,connected_name) != 0 ) return;
        
        //decode( &pack );

        if( is_server ){
          if( strcmp(pack.name_rx,name) != 0 ) return;
        }
        // verifica a chave privada
        if( pack.private_id != data_out.private_id ) return;

        // ----------- validação completa! ------------- //
        Timeout_offline = millis() + offline_timeout;
        Timeout_disconnect = Timeout_offline + disconnect_timeout;
        //Serial.print("[chama uma função que trata dos dados recebidos]\n\n");
        data_in = pack;
        //list_services( &data_in );
        if(callback_recive != nullptr) callback_recive();
        if(handle          != nullptr) handle(EVT_RECIVE);
      }

    }

    //// DECODE ////////////////////////////////////////////////////////////////////////
    //void decode( espnow_device_protocol *data ){
    //  switch( data->package_type ){
    //    case CHALLENGE_PACK:
    //      // descriptografa: de salt até load
    //      remove_salt( (uint8_t*) &(data->salt), 211, data->salt );
    //    break;
    //    default:
    //      // descriptografa: de name até load
    //      remove_salt( (uint8_t*) data->name, sizeof(espnow_device_protocol)-1, data->salt );
    //    break;
    //  }
    //}


    //// SEND ////////////////////////////////////////////////////////////////////////
    void send_raw( uint8_t *load, uint8_t load_len ){
      
      // serviço
      data_out.service_count = 0;
      
      //struct espnow_service<constrain(load_len,0,200-4)> load_struct;

      // load
      load_len = constrain(load_len,0,200);
      memcpy( data_out.load, load, load_len );
      // send
      send();
    }

    void send( ){
      ESPNOWSerial.write( (uint8_t*)&data_out, sizeof(data_out) - (200-load_size) );
      data_out.service_count = 0;
      load_size = 0;
    }

    //// SERVICE //////////////////////////////////////////////////////////////////////////////
    template<uint8_t n>
    bool add_service( struct espnow_service<n> *service ){
      if( (200 - load_size) < (n+4) ) return false;
      data_out.service_count++;
      memcpy(data_out.load+load_size,(uint8_t*)service,2);
      load_size += service.len;
      // send
      return true;
    }

    boolean add_service( uint16_t service, uint8_t *data, uint8_t len ){
        len+=3;
        if( (200 - load_size) < len ) return false;
        data_out.service_count++;
        uint8_t *p = data_out.load+load_size;
        *p = len; p++; // copy len
        p+=buffer_save(p,service); // copy service id
        memcpy( p, data, len-3 );
        load_size += len;
        return true;
    }

    bool list_services( espnow_device_protocol *p ){
        Serial.printf("Services: %i\n", p->service_count );
        int pos = 0;
        for( int i=0;i<p->service_count;i++){
            Serial.printf("[%i] ID: %#04X len: %i\n", i, buffer_get<uint16_t>( p->load+pos+1 ), p->load[pos] );
            pos += p->load[pos];
            if( pos >= 200 ) return false;
        }
        return true;
    }

    bool get_service_by_index( espnow_device_protocol *p, uint8_t n, uint8_t **service ){
        if( p->service_count <= n ) return false;
        int pos = 0;
        for( int i=0;i<n;i++){
            pos += p->load[pos];
            if( pos >= 200 ) return false;
        }
        *service = p->load + pos;
        return true;
    }

    bool get_service( espnow_device_protocol *p, uint16_t service_id, uint8_t **service_return ){
        //if( p->service_count <= n ) return false;
        int pos = 0;
        for(int i=0;i<p->service_count;i++){
            pos += p->load[pos];
            if( pos >= 200 ) return false;
            if( buffer_get<uint16_t>(data_in.load+pos+1) == service_id )
            {
                *service_return = p->load+pos;
                return true;
            }
        }
        return false;
    }

    int get_service( uint16_t service_id, uint8_t **service_return ){
        int pos = 0;
        for(int i=0;i<data_in.service_count;i++){
            pos += data_in.load[pos];
            if( pos >= 200 ) return -1;
            if( buffer_get<uint16_t>(data_in.load+pos+1) == service_id )
            {
                *service_return = data_in.load+pos;
                return data_in.load[pos];
            }
        }
        return -1;
    }

    template < typename T >
    int pass_service( uint16_t service_id, T *service, uint8_t len ){
        int pos = 0;
        for(int i=0;i<data_in.service_count;i++){
          if( buffer_get<uint16_t>(data_in.load+pos+1) == service_id ){
                memcpy( (uint8_t*) service, data_in.load+pos+3, ( data_in.load[pos]-3 > len ? len : data_in.load[pos]-3 ) );
                return (data_in.load[pos]-3)/sizeof(T);
            }
            pos += data_in.load[pos];
            if( pos >= 200 ) return -1;
        }
        return -1;
    }

/*/
    bool list_services( espnow_device_protocol *p ){
        Serial.printf("Services: %i\n", p->service_count );
        int pos = 0;
        for( int i=0;i<p->service_count;i++){
            Serial.printf("[%i] ID: %#04X len: %i\n", i, *((uint16_t*)(p->load+pos+1)), p->load[pos] );
            pos += p->load[pos];
            if( pos >= 200 ) return false;
        }
        return true;
    }
/*/

    //// funções de criptografia /////////////////////////////////////////////////
    uint8_t add_salt( uint8_t *data, uint32_t len ){
      uint8_t salt = random(1,255);
      add_salt( data, len, salt );
      return salt;
    }

    void add_salt( uint8_t *data, uint32_t len, uint8_t salt ){
      for(int i=0;i<len;i++) *(data+i) += salt;
    }

    void remove_salt( uint8_t *data, uint32_t len, uint8_t salt ){
      for(int i=0;i<len;i++) *(data+i) -= salt;
    }


    // update
    void update(){

      boolean Online_now = millis() < Timeout_offline;
      boolean Connect_now = millis() < Timeout_disconnect;

      // scan
      if( mode == SCAN ){
        if( millis() >= scan_timeout ){
          if(handle != nullptr) handle( EVT_SCAN_RESULT );
          //scan( scan_duration );
        }
      }

      // send
      if( (WriteDelay>0) && (mode==CONNECTING||mode==CONNECTED) ){
        if( millis() >= WriteTimeout ){
          WriteTimeout = millis() + WriteDelay;
          data_out.service_count = 0;
          if(callback_send != nullptr) callback_send();
          if(handle != nullptr) handle( EVT_SEND );
          send();
        }
      }

      if( Online_now == Online ){
        Change = false;
      }else{
        Online = Online_now;
        Change = true;
        if(handle != nullptr) handle( Online ? EVT_RISE_CONNECTION : EVT_FALL_CONNECTION );
      }

      if( !Connect_now && mode==CONNECTED ){
        mode = CONNECTING;
        connected_flag = false;
        if(handle != nullptr) handle( EVT_LOST_CONNECTION );
      }

    }

    boolean fall(){ return Change&&(!Online); }
    boolean rise(){ return Change&&Online; }
    boolean change(){ return Change; }
    boolean online(){ return Online; }

    // BASIC ESPNOW FUNCTIONS //////////////////////////////////

    // size_t write(const uint8_t *buffer, size_t size){
    //   send_status = 0;
    //   //Serial.printf("SEND [%i]\n",size);
    //   size_t n = size;
    //   //size_t i = 0;
    //   //int j = 0;

    //   while( n > 0 ){
    //     //uint32_t t = millis();
    //     uint32_t len = ( n > 250 ? 250 : n );
    //     esp_now_send(send_address, buffer, len);
    //     n -= len;
    //     buffer += len;
    //     wait_send();
    //     //Serial.printf("[PACK %i] %i ms\n",j,millis()-t);
    //     //j++;
    //     if(send_dt>0) delay(send_dt);
    //   }
    //   return size;
    // }


    private:
    //
    int find_device( device_t device, device_t *list, uint8_t len ){
      boolean Found = false;
      for(int i=0;i<len;i++){
        if( strcmp(device.name,list[i].name)  != 0 ) continue;
        if( device.utility   != list[i].utility    ) continue;
        if( device.color_rgb != list[i].color_rgb  ) continue;
        if( memcmp( (uint8_t*)(&device.mac), (uint8_t*)(&list[i].mac), 6 ) != 0 ) continue;
        return i;
      }
      return -1;
    }
    
    boolean scan_add_device( espnow_device_protocol *p, const uint8_t * mac){
      if( scan_devices_count >= 20 ) return false;
      device_t device;
      strcpy( device.name, p->name );  // name
      device.color_rgb = p->color_rgb; // color
      device.utility   = p->utility;     // utility
      memcpy( device.mac,  mac,  6 );  // mac
      if( find_device(device,scan_devices,scan_devices_count) == -1 ){
        scan_devices[scan_devices_count] = device;
        scan_devices_count++;
        return true;
      }
      return false;
    }

};

ESPNOW_device ESPNOW_device;

void recive_radio( const uint8_t* mac, const uint8_t* data, int len){
  ESPNOW_device.recive(mac,data,len);
}


#endif