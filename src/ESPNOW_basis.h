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
// Variaveis globais e defines
//========================================================================================

#define ESPNOW_DEVICE__MAX_CONNECTIONS 10
const uint8_t espnow_device__broadcast_mac[6]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
uint8_t espnow_device__channel = 1;

//========================================================================================
// ESPNOW Functions
//========================================================================================

#ifdef ESP32
typedef struct{
  bool set = false;
  esp_now_peer_info_t info;
}espnow_device__esp32_peer_t;

espnow_device__esp32_peer_t peerInfo[ESPNOW_DEVICE__MAX_CONNECTIONS];

espnow_device__esp32_peer_t *espnow_device_get_peer( uint8_t *mac ){
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

    espnow_device__esp32_peer_t *p = nullptr;

    for(int i=0;i<ESPNOW_DEVICE__MAX_CONNECTIONS;i++){
      if( !peerInfo[i].set ) p = peerInfo+i;
    }

    if( p == nullptr ) return false;

    p->set = true;
    //peerInfo->ifidx = (wifi_interface_t) 0;
    memcpy(p->info.peer_addr, mac, 6);
    p->info.channel = espnow_device__channel;
    p->info.encrypt = false;

    // Add peer
    if(esp_now_add_peer(&p->info) != ESP_OK){
      Serial.println("[ESPNOW DEVICE] Failed to add peer");
      return false;
    }

  #else
    esp_now_add_peer( (uint8_t*) mac, ESP_NOW_ROLE_COMBO, espnow_device__channel, NULL, 0);
  #endif

  return true;
}

bool espnow_device_close_peer( uint8_t *mac ){
  //Serial.printf( "[CLOSE][%s]\n", mac2str(mac).c_str() );
  #ifdef ESP32
    espnow_device__esp32_peer_t *p = espnow_device_get_peer( mac );
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