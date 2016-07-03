
#include "stomp.h"
#include "mustang.h"

int 
StompCC::continuous_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = STOMP_STATE;
  cmd.parm2 = 0x03;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;
  
  return amp->continuous_control( cmd );
}

int 
StompCC::discrete_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = STOMP_STATE;
  cmd.parm2 = 0x03;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;

  return amp->discrete_control( cmd );
}

int
StompCC::dispatch( int cc, int value ) {

  switch ( cc ) {
  case 29:
    // Level / Mix / Type
    return cc29( value );
    break;
  case 30:
    // Gain / Freq / Sens / Thresh
    return cc30( value );
    break;
  case 31:
    // Low / Heel Freq / Octave / Sens / Ratio
    return cc31( value );
    break;
  case 32:
    // Mid / Toe Freq / Low / Octave / Attack
    return cc32( value );
    break;
  case 33:
    // High / High Q / Peak / Relase 
    return cc33( value );
    break;
  default:
    return 0;
    break;
  }
}

