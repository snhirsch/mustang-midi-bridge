
#include "reverb.h"
#include "mustang.h"

int 
ReverbCC::efx_common1(int parm, int bucket, int type, int value) {
  return amp->efx_common1( parm, bucket, type, value );
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
  return efx_common1( 0x00, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc60( int value ) {
  return efx_common1( 0x01, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc61( int value ) {
  return efx_common1( 0x02, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc62( int value ) {
  return efx_common1( 0x03, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc63( int value ) {
  return efx_common1( 0x04, 0x0b, REVERB_STATE, value );
}

