
#include "delay.h"
#include "mustang.h"
#include "constants.h"

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

