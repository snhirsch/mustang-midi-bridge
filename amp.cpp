
#include "amp.h"
#include "magic.h"

int 
AmpCC::continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x02;
  memcpy( cmd + 3, model, 2 );
  cmd[5] = parm5;
  cmd[6] = parm6;
  cmd[7] = parm7;

  unsigned short magic = magic_values[value];
  cmd[9] = magic & 0xff;
  cmd[10] = (magic >> 8) & 0xff;
  return 0;
}

int 
AmpCC::discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x02;
  memcpy( cmd + 3, model, 2 );
  cmd[5] = parm5;
  cmd[6] = parm6;
  cmd[7] = parm7;

  cmd[9] = value;
  return 0;
}

int
AmpCC::dispatch( int cc, int value, unsigned char *cmd ) {

  switch( cc ) {
  case 69:
    // Gain
    return cc69( value, cmd );
    break;
  case 70:
    // Channel volume
    return cc70( value, cmd );
    break;
  case 71:
    // Treble
    return cc71( value, cmd );
    break;
  case 72:
    // Mid
    return cc72( value, cmd );
    break;
  case 73:
    // Bass
    return cc73( value, cmd );
    break;
  case 74:
    // Sag
    return cc74( value, cmd );
    break;
  case 75:
    // Bias
    return cc75( value, cmd );
    break;
  case 76:
    // Noise Gate
    return cc76( value, cmd );
    break;
  case 77:
    // Cabinet
    return cc77( value, cmd );
    break;
  case 78:
    // Presence / Gain2 / Cut
    return cc78( value, cmd );
    break;
  case 79:
    // Blend / Master Volume
    return cc79( value, cmd );
    break;
  default:
    return -1;
    break;
  }
}

