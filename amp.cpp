
#include "amp.h"
#include "mustang.h"

int 
AmpCC::continuous_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = AMP_STATE;
  cmd.parm2 = 0x02;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;
  
  return amp->continuous_control( cmd );
}

int 
AmpCC::discrete_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = AMP_STATE;
  cmd.parm2 = 0x02;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;

  return amp->discrete_control( cmd );
}

int
AmpCC::dispatch( int cc, int value ) {

  switch( cc ) {
  case 69:
    // Gain
    return cc69( value );
    break;
  case 70:
    // Channel volume
    return cc70( value );
    break;
  case 71:
    // Treble
    return cc71( value );
    break;
  case 72:
    // Mid
    return cc72( value );
    break;
  case 73:
    // Bass
    return cc73( value );
    break;
  case 74:
    // Sag
    return cc74( value );
    break;
  case 75:
    // Bias
    return cc75( value );
    break;
  case 76:
    // Noise Gate
    return cc76( value );
    break;
  case 77:
    // Cabinet
    return cc77( value );
    break;
  case 78:
    // Presence / Gain2 / Cut
    return cc78( value );
    break;
  case 79:
    // Blend / Master Volume
    return cc79( value );
    break;
  default:
    return 0;
    break;
  }
}

