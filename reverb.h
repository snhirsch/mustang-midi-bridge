// -*-c++-*-

#ifndef _REVERB_H
#define _REVERB_H

class Mustang;

class ReverbCC {

protected:
  
  Mustang * amp;

public:

  ReverbCC( Mustang * theAmp ) : amp(theAmp) {}

  // Level
  int cc59( int value );
  // Decay
  int cc60( int value );
  // Dwell
  int cc61( int value );
  // Diffusion
  int cc62( int value );
  // Tone
  int cc63( int value );
};

#endif
