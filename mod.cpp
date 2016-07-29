
#include "mod.h"
#include "magic.h"

int 
ModCC::continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x04;
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
ModCC::discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x04;
  memcpy( cmd + 3, model, 2 );
  cmd[5] = parm5;
  cmd[6] = parm6;
  cmd[7] = parm7;

  cmd[9] = value;
  return 0;
}

int
ModCC::dispatch( int cc, int value, unsigned char *cmd ) {

  switch ( cc ) {
  case 39:
    // Level / Mix
    return cc39( value, cmd );
    break;
  case 40:
    // Rate / Rotor / Freq / Pitch
    return cc40( value, cmd );
    break;
  case 41:
    // Depth / Duty Cycle / Resonance / Detune / Heel Freq
    return cc41( value, cmd );
    break;
  case 42:
    // Delay / Fdbk / LFO / Min Freq / Feedback / Toe Freq
    return cc42( value, cmd );
    break;
  case 43:
    // LR Phase / Release / Tri Shape / PreDelay / High Q 
    return cc43( value, cmd );
    break;
  default:
    return 0;
    break;
  }
}

