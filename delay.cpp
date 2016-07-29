
#include "delay.h"
#include "magic.h"

int 
DelayCC::continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x05;
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
DelayCC::discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd ) {
  cmd[2] = 0x05;
  memcpy( cmd + 3, model, 2 );
  cmd[5] = parm5;
  cmd[6] = parm6;
  cmd[7] = parm7;

  cmd[9] = value;
  return 0;
}

int
DelayCC::dispatch( int cc, int value, unsigned char *cmd ) {

  switch ( cc ) {
  case 49:
    // Level
    return cc49( value, cmd );
    break;
  case 50:
    // Delay Time
    return cc50( value, cmd );
    break;
  case 51:
    // Feedback / FFdbk
    return cc51( value, cmd );
    break;
  case 52:
    // Brightness / Frequency / Release / RFdbk / Flutter
    return cc52( value, cmd );
    break;
  case 53:
    // Attenuation / Resonance / Mode / Stereo / Threshold 
    // Tone / Brightness / Separation
    return cc53( value, cmd );
    break;
  case 54:
    // Input Level / Stereo / Brightness
    return cc54( value, cmd );
    break;
  default:
    return 0;
    break;
  }
}

