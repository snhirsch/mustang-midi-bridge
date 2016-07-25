// -*-c++-*-

#ifndef _REVERB_H
#define _REVERB_H

#include <cstring>

class Mustang;

class ReverbCC {

protected:
  Mustang * amp;
  unsigned char model[2];
  unsigned char slot;

  int continuous_control( int parm5, int parm6, int parm7, int value );

public:
  ReverbCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : 
    amp(theAmp), 
    slot(theSlot) 
  {
    memcpy( this->model, model, 2 );
  }

  int dispatch( int cc, int value );
  const unsigned char *getModel( void ) { return model;}
  const unsigned char getSlot( void ) { return slot;}

private:
  // Level
  int cc59( int value ) { return continuous_control( 0x00, 0x00, 0x0b, value );}
  // Decay
  int cc60( int value ) { return continuous_control( 0x01, 0x01, 0x0b, value );}
  // Dwell
  int cc61( int value ) { return continuous_control( 0x02, 0x02, 0x0b, value );}
  // Diffusion
  int cc62( int value ) { return continuous_control( 0x03, 0x03, 0x0b, value );}
  // Tone
  int cc63( int value ) { return continuous_control( 0x04, 0x04, 0x0b, value );}
};


class NullReverbCC : public ReverbCC {
public:
  NullReverbCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : ReverbCC(theAmp,model,theSlot) {}
private:
  int cc59( int value ) { return 0;}
  int cc60( int value ) { return 0;}
  int cc61( int value ) { return 0;}
  int cc62( int value ) { return 0;}
  int cc63( int value ) { return 0;}
};


#endif
