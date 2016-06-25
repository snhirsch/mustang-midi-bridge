
#include "amp.h"
#include "mustang.h"

int 
AmpCC::control_common1( int parm, int bucket, int value ) {
  return amp->control_common1( parm, bucket, value );
}

int 
AmpCC::control_common2( int parm, int bucket, int value ) {
  return amp->control_common2( parm, bucket, value );
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

int 
AmpCC::cc69( int value ) {
  return control_common1( 0x01, 0x0c, value );
}

int 
AmpCC::cc70( int value ) {
  return control_common1( 0x00, 0x0c, value );
}

int 
AmpCC::cc71( int value ) {
  return control_common1( 0x04, 0x0c, value );
}

int 
AmpCC::cc72( int value ) {
  return control_common1( 0x05, 0x0c, value );
}

int 
AmpCC::cc73( int value ) {
  return control_common1( 0x06, 0x0c, value );
}

int 
AmpCC::cc74( int value ) {
  if ( value > 2 ) return 0;
  return control_common2( 0x13, 0x8f, value );
}

int 
AmpCC::cc75( int value ) {
  return control_common1( 0x0a, 0x0d, value );
}

int 
AmpCC::cc76( int value ) {
  if ( value > 4 ) return 0;
  return control_common2( 0x0f, 0x90, value );
}

int 
AmpCC::cc77( int value ) {
  if ( value < 1 || value > 12 ) return 0;
  return control_common2( 0x11, 0x8e, value );
}



int 
AmpCC1::cc78( int value ) {
  return control_common1( 0x07, 0x0c, value );
}

int 
AmpCC1::cc79( int value ) {
  return control_common1( 0x02, 0x0c, value );
}
  


int 
AmpCC2::cc78( int value ) {
  return control_common1( 0x02, 0x0c, value );
}

int 
AmpCC2::cc79( int value ) {
  return control_common1( 0x03, 0x0c, value );
}

  

int 
AmpCC3::cc78( int value ) {
  return control_common1( 0x07, 0x0c, value );
}

int 
AmpCC3::cc79( int value ) {
  return control_common1( 0x03, 0x0c, value );
}
  


int 
AmpCC4::cc78( int value ) {
  return control_common1( 0x07, 0x0c, value );
}

int 
AmpCC4::cc79( int value ) {
  return control_common1( 0x03, 0x0c, value );
}
