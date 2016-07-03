// -*-c++-*-

#ifndef _REVERB_H
#define _REVERB_H

class Mustang;

class ReverbCC {

protected:
  Mustang * amp;

  int continuous_control( int parm5, int parm6, int parm7, int value );

public:
  ReverbCC( Mustang * theAmp ) : amp(theAmp) {}

  int dispatch( int cc, int value );

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
  NullReverbCC( Mustang * theAmp ) : ReverbCC(theAmp) {}
private:
  int cc59( int value ) { return 0;}
  int cc60( int value ) { return 0;}
  int cc61( int value ) { return 0;}
  int cc62( int value ) { return 0;}
  int cc63( int value ) { return 0;}
};


#endif
