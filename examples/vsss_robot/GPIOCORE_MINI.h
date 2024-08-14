
//==========================================================================
// GPIO (stand alone GPIO_CORE lib)
//==========================================================================

float mapf( float in, float in_down, float in_up, float out_down, float out_up ){
  return ( in_down == in_up ? out_down : out_down + (in-in_down)*(out_up-out_down)/(float)(in_up-in_down) );
}

/*/
  4 tipos de entradas de sinal:
  - leitura de sinal digital
  - leitura de sinal analogico
  - leitura de botão
  - leitura de tensão
/*/

class GPIO_INPUT{

  private:
  
    bool    EN = false;
    uint8_t Pin = 0xff;
    bool    PIN_IS_DIG = true;
    
    bool State_on = false;
    bool State    = false;
    bool Change   = false;

    uint16_t Trig = 2048;

    uint32_t Debounce_timeout    = 0;
    uint32_t Debounce_delay      = 50;
    bool     Debounce_state_last = LOW;
    bool     Debounce_active     = false;

    uint32_t State_start_ms_last = 0;
    uint32_t State_start_ms = 0;

    float Voltage_low  = 0.0;
    float Voltage_high = 3.3;
  
  public:

  enum GPIO_TYPE{
    GPIO_DIGITAL,
    GPIO_DIGITAL_PULLUP,
    GPIO_BUTTON,
    GPIO_BUTTON_PULLUP,
    GPIO_ANALOG,
    GPIO_VOLTAGE
  };

  GPIO_TYPE Type = GPIO_DIGITAL;

  GPIO_INPUT(uint8_t pin, boolean state_on, GPIO_TYPE type){
    EN = true;
    Pin = pin;
    State_on = state_on;
    Type = type;
    if( Type == GPIO_BUTTON || Type == GPIO_BUTTON_PULLUP ) Debounce_active = true;
    PIN_IS_DIG = !( Type == GPIO_ANALOG || Type == GPIO_VOLTAGE );
  }
  GPIO_INPUT(uint8_t pin, GPIO_TYPE type){
    GPIO_INPUT(pin, HIGH, type);
  }
  GPIO_INPUT(uint8_t pin){
    GPIO_INPUT(pin, HIGH, GPIO_DIGITAL);
  }

  // begin
  
  void begin(){
    State_start_ms = millis();
    pinMode(Pin, ( Type == GPIO_BUTTON_PULLUP || Type == GPIO_DIGITAL_PULLUP ) ? INPUT_PULLUP : INPUT );
  }

  void set_trigger(uint16_t trig){
    Trig = trig;
  }

  void set_voltage_range( float voltage_low, float voltage_high, float voltage_trig ){
    Voltage_low  = voltage_low;
    Voltage_high = voltage_high;
    Trig = constrain( mapf( voltage_trig, Voltage_low, Voltage_high, 0, 4095 ), 0, 4095 );
  }

  // raw functions
  
  bool digital_read(){
    return ( EN ? ( PIN_IS_DIG ? digitalRead(Pin) : analogRead(Pin)>=Trig ) : 0 );
  }

  int analog_read(){
    return ( EN ? ( PIN_IS_DIG ? (digitalRead(Pin)?4095:0) : analogRead(Pin) ) : 0 );
  }

  // state functions

  bool change(){
    return Change;
  }

  bool rise(){
    return State && Change;
  }

  bool fall(){
    return (!State) && Change;
  }

  bool isOnFor( uint32_t ms ){
    return State && ( (millis()-State_start_ms) >= ms );
  }
  
  bool isOffFor( uint32_t ms ){
    return (!State) && ( (millis()-State_start_ms) >= ms );
  }

  bool wasOnFor( uint32_t ms ){
    return (!State) && Change && ( (State_start_ms-State_start_ms_last) >= ms );
  }
  
  bool wasOffFor( uint32_t ms ){
    return State && Change &&( (State_start_ms-State_start_ms_last) >= ms );
  }

  bool isOff(){
    return !isOn();
  }

  bool isOn(){
    return State;
  }

  // update
  bool update(){
    
    if( !EN ) return false;
    
    Change = false;
    bool state_act = ( State_on == digital_read() );
    
    if( Debounce_active ){

      if( Debounce_state_last != state_act ){
        Debounce_state_last = state_act;
        Debounce_timeout = millis() + Debounce_delay;
      }else if( State != state_act ){
        if( Debounce_timeout <= millis() ){
          State_start_ms_last = State_start_ms;
          State_start_ms = millis();
          State = state_act;
          Change = true;
        }
      }

    }else{
      Change = (State != state_act);
      State = state_act;
    }
    return State;
  }

  // init debounce
  void init_debounce( uint16_t debounce_delay ){
    Debounce_active = true;
    Debounce_delay  = debounce_delay;
  }

  void deinit_debounce(){
    Debounce_active = false;
  }

  // voltage
  float voltage(){
    return mapf( analog_read(), 0, 4095, Voltage_low, Voltage_high );
  }

};

