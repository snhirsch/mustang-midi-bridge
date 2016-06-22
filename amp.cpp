
#include "amp.h"
#include "mustang.h"

int 
AmpCC::cc69( int value ) {
  return amp->control_common1( 0x01, 0x0c, value );
}

int 
AmpCC::cc70( int value ) {
  return amp->control_common1( 0x00, 0x0c, value );
}

int 
AmpCC::cc71( int value ) {
  return amp->control_common1( 0x04, 0x0c, value );
}

int 
AmpCC::cc72( int value ) {
  return amp->control_common1( 0x05, 0x0c, value );
}

int 
AmpCC::cc73( int value ) {
  return amp->control_common1( 0x06, 0x0c, value );
}

int 
AmpCC::cc74( int value ) {
  if ( value > 2 ) return 0;
  return amp->control_common2( 0x13, 0x8f, value );
}

int 
AmpCC::cc75( int value ) {
  return amp->control_common1( 0x0a, 0x0d, value );
}

int 
AmpCC::cc76( int value ) {
  if ( value > 4 ) return 0;
  return amp->control_common2( 0x0f, 0x90, value );
}

int 
AmpCC::cc77( int value ) {
  if ( value < 1 || value > 12 ) return 0;
  return amp->control_common2( 0x11, 0x8e, value );
}



int 
AmpCC1::cc78( int value ) {
  return amp->control_common1( 0x07, 0x0c, value );
}

int 
AmpCC1::cc79( int value ) {
  return amp->control_common1( 0x02, 0x0c, value );
}
  


int 
AmpCC2::cc78( int value ) {
  return amp->control_common1( 0x02, 0x0c, value );
}

int 
AmpCC2::cc79( int value ) {
  return amp->control_common1( 0x03, 0x0c, value );
}

  

int 
AmpCC3::cc78( int value ) {
  return amp->control_common1( 0x07, 0x0c, value );
}

int 
AmpCC3::cc79( int value ) {
  return amp->control_common1( 0x03, 0x0c, value );
}
  


int 
AmpCC4::cc78( int value ) {
  return amp->control_common1( 0x07, 0x0c, value );
}

int 
AmpCC4::cc79( int value ) {
  return amp->control_common1( 0x03, 0x0c, value );
}
