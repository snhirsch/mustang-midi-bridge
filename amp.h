// -*-c++-*-

#ifndef _AMPCC_H
#define _AMPCC_H

class Mustang;

// F57 Deluxe
// F57 Champ
// F65 Deluxe
// F65 Princeton
// F65 Twin
//
class AmpCC {

protected:
  
  Mustang * amp;

public:

  AmpCC( Mustang * theAmp ) : amp(theAmp) {}

  // Gain
  int cc69( int value );
  // Ch. Volume
  int cc70( int value );
  // Treble
  int cc71( int value );
  // Mid
  int cc72( int value );
  // Bass
  int cc73( int value );
  // Sag
  virtual int cc74( int value );
  // Bias
  virtual int cc75( int value );
  // Noise Gate
  int cc76( int value );
  // Cabinet
  int cc77( int value );

  // Dummy in base class
  virtual int cc78( int value ) { return 0;}
  virtual int cc79( int value ) { return 0;}
};


// F59 Bassman
// British 70s
//
class AmpCC1 : public AmpCC {
public:
  AmpCC1( Mustang * theAmp ) : AmpCC(theAmp) {}
  // Presence
  virtual int cc78( int value );
  // Blend
  virtual int cc79( int value );
};
  

// Fender Supersonic
//
class AmpCC2 : public AmpCC {
public:
  AmpCC2( Mustang * theAmp ) : AmpCC(theAmp) {}
  // Gain2
  virtual int cc78( int value );
  // Master Volume
  virtual int cc79( int value );
};
  

// British 60s
//
class AmpCC3 : public AmpCC {
public:
  AmpCC3( Mustang * theAmp ) : AmpCC(theAmp) {}
  // Cut
  virtual int cc78( int value );
  // Master Volume
  virtual int cc79( int value );
};
  

// British 80s
// American 90s
// Metal 2000
//
class AmpCC4 : public AmpCC {
public:
  AmpCC4( Mustang * theAmp ) : AmpCC(theAmp) {}
  // Presence
  virtual int cc78( int value );
  // Master Volume
  virtual int cc79( int value );
};
  

// Studio Preamp
//
class AmpCC5 : public AmpCC {
public:
  AmpCC5( Mustang * theAmp ) : AmpCC(theAmp) {}
  virtual int cc78( int value ) { return 0;}
  virtual int cc79( int value ) { return 0;}
};
  
#endif
