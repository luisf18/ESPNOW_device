/* ====================================================
 *  PIXEL Library
 *    Author: luisf18 (github)
 *    Ver.: 1.0.0
 *    last_update: 14/11/2022
 * ====================================================
 */

 // Note: This example uses Neopixel LED board, 32 LEDs chained one
//      after another, each RGB LED has its 24 bit value 
//      for color configuration (8b for each color)
//
//      Bits encoded as pulses as follows:
//
//      "0":
//         +-------+              +--
//         |       |              |
//         |       |              |
//         |       |              |
//      ---|       |--------------|
//         +       +              +
//         | 0.4us |   0.85 0us   |
//
//      "1":
//         +-------------+       +--
//         |             |       |
//         |             |       |
//         |             |       |
//         |             |       |
//      ---+             +-------+
//         |    0.8us    | 0.4us |

#ifndef PIXEL_H
#define PIXEL_H
#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp32-hal.h"

#define PX_BLACK  0x000000
#define PX_GREEN  0x00FF00
#define PX_BLUE   0x0000FF
#define PX_RED    0xFF0000
#define PX_PURPLE 0xFF00FF
#define PX_YELLOW 0xFFFF00
//#define PX_YELLOW 0xFF00FF
#define PX_WHITE  0xFFFFFF

#define RGB_565(r,g,b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )

#define RGB_565(r,g,b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )

#define RGB_24(r,g,b) ( (r << 16) | (g << 8) | b )
#define GBR_24(g,b,r) ( (g << 16) | (b << 8) | r )

#define PX_LEDS 8
#define PX_BITS 24*PX_LEDS

class Pixel{

  private:
    int     pin        = -1;
    int     LEDS       = -1;
    uint8_t Brightness = 255;
    // int     BITS       = -1;
    rmt_data_t DATA[PX_BITS];
    rmt_obj_t* rmt_send = NULL;
    // uint32_t   Timeout[PX_LEDS];
    // uint16_t   T_ms[PX_LEDS];
    // uint32_t   Color_0[PX_LEDS];
    // uint32_t   Color_1[PX_LEDS];
    // uint8_t    Blink_n[PX_LEDS];

  public:
    
    Pixel(uint8_t _pin, uint16_t _leds) { pin = _pin; LEDS = _leds; };
    boolean begin(){
      if ((rmt_send = rmtInit(pin, true, RMT_MEM_64)) == NULL){
          Serial.println("init sender failed\n");
      }
      float realTick = rmtSetTick(rmt_send, 100);
      Serial.printf("real tick set to: %fns\n", realTick);
      return true;
    };
    
    boolean stop(){return rmtDeinit(rmt_send);};
    
    boolean write(){ return rmtWrite(rmt_send, DATA, PX_BITS); };
    //void set(uint32_t color){ Serial.print(rmtWrite(rmt_send, DATA, PX_BITS)); }
    
    void set(uint32_t color, uint16_t pixel){
      if( pixel >= LEDS )return;
      int i = pixel*24;

      byte R = ( color & 0xff0000 ) >> 16;
      byte G = ( color & 0x00ff00 ) >> 8;
      byte B = ( color & 0x0000ff ) >> 0;

      R = map( R, 0, 255, 0, Brightness );
      G = map( G, 0, 255, 0, Brightness );
      B = map( B, 0, 255, 0, Brightness );

      color = ( G << 16 ) | ( R << 8 ) | ( B );

      for(int bit=0;bit<24;bit++){
        if ( color & (1<<(23-bit)) ) {
            DATA[i].level0 = 1;
            DATA[i].duration0 = 8;
            DATA[i].level1 = 0;
            DATA[i].duration1 = 4;
        } else {
            DATA[i].level0 = 1;
            DATA[i].duration0 = 4;
            DATA[i].level1 = 0;
            DATA[i].duration1 = 8;
        }
        i++;
      }
    }

    void fill( uint32_t color ){ for( int i=0;i<LEDS;i++ ) set( color, i ); }

    void clear(){ fill(0); }

    void push( uint32_t *buf, uint16_t len ){ for( int i=0;(i<LEDS)&&(i<len);i++ ) set( buf[i], i ); }

    void setBrightness( uint8_t x ){ Brightness = x; }

    uint32_t colors[ 7 ] = {
      0xff0000,
      0x00ff00,
      0x0000ff,
      0xffff00,
      0x00ffff,
      0xff00ff,
      0xffffff,
    };

    //// Blink
    //void set_blink(uint32_t color, int pixel, int t_ms){ if( pixel >= LEDS )return; set(color,pixel); T_ms[pixel] = t_ms; timeout[pixel] = millis()+T_ms[pixel]; };
    //void update(uint32_t color, int pixel, int t_ms){
    //  for(int i=0;i<LEDS;i++){
    //    if( Blink_n[i] > 0 ){
    //      if( timeout[pixel] < millis() ){
    //        timeout[pixel] = millis()+T_ms[pixel];
    //        if( Blink_n[i] == 1 ){
    //          set(Color_0[i],i);
    //        }
    //        Blink_n[i] = ( Blink_n[i] == 1 ? 2 : 2 );
    //      }
//
    //    }
    //  }
    //};
    
};

#endif
