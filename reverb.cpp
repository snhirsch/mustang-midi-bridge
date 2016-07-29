
#include "reverb.h"
#include "magic.h"

int 
ReverbCC::continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x06;
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
ReverbCC::dispatch( int cc, int value, unsigned char *cmd ) {

  switch ( cc ) {
  case 59:
    // Level
    return cc59( value, cmd );
    break;
  case 60:
    // Decay
    return cc60( value, cmd );
    break;
  case 61:
    // Dwell
    return cc61( value, cmd );
    break;
  case 62:
    // Diffusion
    return cc62( value, cmd );
    break;
  case 63:
    // Tone
    return cc63( value, cmd );
    break;
  default:
    return 0;
    break;
  }
}

