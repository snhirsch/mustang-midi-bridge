
#include "reverb.h"
#include "mustang.h"

int 
ReverbCC::cc59( int value ) {
  return amp->efx_common1( 0x00, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc60( int value ) {
  return amp->efx_common1( 0x01, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc61( int value ) {
  return amp->efx_common1( 0x02, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc62( int value ) {
  return amp->efx_common1( 0x03, 0x0b, REVERB_STATE, value );
}

int 
ReverbCC::cc63( int value ) {
  return amp->efx_common1( 0x04, 0x0b, REVERB_STATE, value );
}

