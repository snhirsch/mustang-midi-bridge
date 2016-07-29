
#include "stomp.h"
#include "magic.h"

int 
StompCC::continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x03;
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
StompCC::discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x03;
  memcpy( cmd + 3, model, 2 );
  cmd[5] = parm5;
  cmd[6] = parm6;
  cmd[7] = parm7;

  cmd[9] = value;
  return 0;
}

int
StompCC::dispatch( int cc, int value, unsigned char *cmd ) {

  switch ( cc ) {
  case 29:
    // Level / Mix / Type
    return cc29( value, cmd );
    break;
  case 30:
    // Gain / Freq / Sens / Thresh
    return cc30( value, cmd );
    break;
  case 31:
    // Low / Heel Freq / Octave / Sens / Ratio
    return cc31( value, cmd );
    break;
  case 32:
    // Mid / Toe Freq / Low / Octave / Attack
    return cc32( value, cmd );
    break;
  case 33:
    // High / High Q / Peak / Relase 
    return cc33( value, cmd );
    break;
  default:
    return 0;
    break;
  }
}

