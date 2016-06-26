
#include "delay.h"
#include "mustang.h"

int 
DelayCC::continuous_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = DELAY_STATE;
  cmd.parm2 = 0x05;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;
  
  return amp->continuous_control( cmd );
}

int 
DelayCC::discrete_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = DELAY_STATE;
  cmd.parm2 = 0x05;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;

  return amp->discrete_control( cmd );
}

int
DelayCC::dispatch( int cc, int value ) {

  switch ( cc ) {
  case 49:
    // Level
    return cc49( value );
    break;
  case 50:
    // Delay Time
    return cc50( value );
    break;
  case 51:
    // Feedback / FFdbk
    return cc51( value );
    break;
  case 52:
    // Brightness / Frequency / Release / RFdbk / Flutter
    return cc52( value );
    break;
  case 53:
    // Attenuation / Resonance / Mode / Stereo / Threshold 
    // Tone / Brightness / Separation
    return cc53( value );
    break;
  case 54:
    // Input Level / Stereo / Brightness
    return cc54( value );
    break;
  default:
    return 0;
    break;
  }
}

int 
DelayCC::cc49( int value ) {
  return continuous_control( 0x00, 0x00, 0x01, value );
}

int 
DelayCC::cc50( int value ) {
  return continuous_control( 0x01, 0x01, 0x06, value );
}



int 
MonoDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
MonoDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
MonoDelayCC::cc53( int value ) {
  return continuous_control( 0x04, 0x04, 0x01, value );
}



int 
EchoFilterCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
EchoFilterCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
EchoFilterCC::cc53( int value ) {
  return continuous_control( 0x04, 0x04, 0x01, value );
}

int 
EchoFilterCC::cc54( int value ) {
  return continuous_control( 0x05, 0x05, 0x01, value );
}



int 
MultitapDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
MultitapDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
MultitapDelayCC::cc53( int value ) {
  if ( value > 3 ) return 0;
  return discrete_control( 0x04, 0x04, 0x8b, value );
}



int 
PingPongDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
PingPongDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
PingPongDelayCC::cc53( int value ) {
  return continuous_control( 0x04, 0x04, 0x01, value );
}



int 
DuckingDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
DuckingDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
DuckingDelayCC::cc53( int value ) {
  return continuous_control( 0x04, 0x04, 0x01, value );
}



int 
ReverseDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
ReverseDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
ReverseDelayCC::cc53( int value ) {
  return continuous_control( 0x04, 0x04, 0x01, value );
}



int 
TapeDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
TapeDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
TapeDelayCC::cc53( int value ) {
  return continuous_control( 0x04, 0x04, 0x01, value );
}

int 
TapeDelayCC::cc54( int value ) {
  return continuous_control( 0x05, 0x05, 0x01, value );
}



int 
StereoTapeDelayCC::cc51( int value ) {
  return continuous_control( 0x02, 0x02, 0x01, value );
}

int 
StereoTapeDelayCC::cc52( int value ) {
  return continuous_control( 0x03, 0x03, 0x01, value );
}

int 
StereoTapeDelayCC::cc53( int value ) {
  return continuous_control( 0x04, 0x05, 0x01, value );
}

int 
StereoTapeDelayCC::cc54( int value ) {
  return continuous_control( 0x05, 0x04, 0x01, value );
}



