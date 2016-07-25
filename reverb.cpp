
#include "reverb.h"
#include "mustang.h"
#include "constants.h"

int 
ReverbCC::continuous_control( int parm5, int parm6, int parm7, int value ) {
  Mustang::Cmd cmd;
  cmd.state_index = REVERB_STATE;
  cmd.parm2 = 0x06;
  cmd.parm5 = parm5;
  cmd.parm6 = parm6;
  cmd.parm7 = parm7;
  cmd.value = value;
  
  return amp->continuous_control( cmd );
}

int
ReverbCC::dispatch( int cc, int value ) {

  switch ( cc ) {
  case 59:
    // Level
    return cc59( value );
    break;
  case 60:
    // Decay
    return cc60( value );
    break;
  case 61:
    // Dwell
    return cc61( value );
    break;
  case 62:
    // Diffusion
    return cc62( value );
    break;
  case 63:
    // Tone
    return cc63( value );
    break;
  default:
    return 0;
    break;
  }
}

