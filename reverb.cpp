
#include "reverb.h"
#include "mustang.h"

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

int 
ReverbCC::cc59( int value ) {
  return continuous_control( 0x00, 0x00, 0x0b, value );
}

int 
ReverbCC::cc60( int value ) {
  return continuous_control( 0x01, 0x01, 0x0b, value );
}

int 
ReverbCC::cc61( int value ) {
  return continuous_control( 0x02, 0x02, 0x0b, value );
}

int 
ReverbCC::cc62( int value ) {
  return continuous_control( 0x03, 0x03, 0x0b, value );
}

int 
ReverbCC::cc63( int value ) {
  return continuous_control( 0x04, 0x04, 0x0b, value );
}

