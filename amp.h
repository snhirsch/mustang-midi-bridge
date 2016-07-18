// -*-c++-*-

#ifndef _AMPCC_H
#define _AMPCC_H

class Mustang;

// F57 Deluxe
// F57 Champ
// F65 Deluxe
// F65 Princeton
// F65 Twin
// 60s Thrift
//
class AmpCC {

protected:
  Mustang * amp;

  // Only base class is friend of Mustang, so forward calls from
  // derived classes through these methods.
  int continuous_control( int parm5, int parm6, int parm7, int value );
  int discrete_control( int parm5, int parm6, int parm7, int value );

public:
  AmpCC( Mustang * theAmp ) : amp(theAmp) {}

  virtual int dispatch( int cc, int value );

private:
  // Gain
  virtual int cc69( int value ) { return continuous_control( 0x01, 0x01, 0x0c, value );}
  // Ch. Volume
  virtual int cc70( int value ) { return continuous_control( 0x00, 0x00, 0x0c, value );}
  // Treble
  virtual int cc71( int value ) { return continuous_control( 0x04, 0x04, 0x0c, value );}
  // Mid
  virtual int cc72( int value ) { return continuous_control( 0x05, 0x05, 0x0c, value );}
  // Bass
  virtual int cc73( int value ) { return continuous_control( 0x06, 0x06, 0x0c, value );}
  // Sag
  virtual int cc74( int value ) { 
    if ( value > 2 ) return 0;
    else             return discrete_control( 0x13, 0x13, 0x8f, value );
  }
  // Bias
  virtual int cc75( int value ) { return continuous_control( 0x0a, 0x0a, 0x0d, value );}
  // Noise Gate
  virtual int cc76( int value ) { 
    if ( value > 4 ) return 0;
    else             return discrete_control( 0x0f, 0x0f, 0x90, value );
  }
  // Cabinet
  virtual int cc77( int value ) {
    if ( value > 12 ) return 0;
    else              return discrete_control( 0x11, 0x11, 0x8e, value );
  }

  // Dummy in base class
  virtual int cc78( int value ) { return 0;}
  virtual int cc79( int value ) { return 0;}

  // Noise Gate Custom Threshold
  virtual int cc90( int value ) { 
    if ( value > 9 ) return 0;
    else             return discrete_control( 0x10, 0x10, 0x86, value );
  }
  // Noise Gate Custom Depth
  virtual int cc91( int value ) { return continuous_control( 0x09, 0x09, 0x0c, value );}
};


// F59 Bassman
// British 70s
//
class AmpCC1 : public AmpCC {
public:
  AmpCC1( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // Presence
  virtual int cc78( int value ) { return continuous_control( 0x07, 0x07, 0x0c, value );}
  // Blend
  virtual int cc79( int value ) { return continuous_control( 0x02, 0x02, 0x0c, value );}
};
  

// Fender Supersonic
//
class AmpCC2 : public AmpCC {
public:
  AmpCC2( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // Gain2
  virtual int cc78( int value ) { return continuous_control( 0x02, 0x02, 0x0c, value );}
  // Master Volume
  virtual int cc79( int value ) { return continuous_control( 0x03, 0x03, 0x0c, value );}
};
  

// British 60s
//
class AmpCC3 : public AmpCC {
public:
  AmpCC3( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // Cut
  virtual int cc78( int value ) { return continuous_control( 0x07, 0x07, 0x0c, value );}
  // Master Volume
  virtual int cc79( int value ) { return continuous_control( 0x03, 0x03, 0x0c, value );}
};
  

// British 80s
// American 90s
// Metal 2000
// British Watt
//
class AmpCC4 : public AmpCC {
public:
  AmpCC4( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // Presence
  virtual int cc78( int value ) { return continuous_control( 0x07, 0x07, 0x0c, value );}
  // Master Volume
  virtual int cc79( int value ) { return continuous_control( 0x03, 0x03, 0x0c, value );}
};
  

// Studio Preamp
//
class AmpCC5 : public AmpCC {
public:
  AmpCC5( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // No sag / bias
  virtual int cc74( int value ) { return 0;}
  virtual int cc75( int value ) { return 0;}
  // No pres / master
  virtual int cc78( int value ) { return 0;}
  virtual int cc79( int value ) { return 0;}
};
  

// British Color
//
class AmpCC6 : public AmpCC {
public:
  AmpCC6( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // Master Volume
  virtual int cc79( int value ) { return continuous_control( 0x03, 0x03, 0x0c, value );}
};
  

// F57 Twin
//
class AmpCC7 : public AmpCC {
public:
  AmpCC7( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  // Presence
  virtual int cc78( int value ) { return continuous_control( 0x07, 0x07, 0x0c, value );}
};
  

// Null Amp
//
class NullAmpCC : public AmpCC {
public:
  NullAmpCC( Mustang * theAmp ) : AmpCC(theAmp) {}
private:
  virtual int cc69( int value ) { return 0;}
  virtual int cc70( int value ) { return 0;}
  virtual int cc71( int value ) { return 0;}
  virtual int cc72( int value ) { return 0;}
  virtual int cc73( int value ) { return 0;}
  virtual int cc74( int value ) { return 0;}
  virtual int cc75( int value ) { return 0;}
  virtual int cc76( int value ) { return 0;}
  virtual int cc77( int value ) { return 0;}
  virtual int cc78( int value ) { return 0;}
  virtual int cc79( int value ) { return 0;}
};
  
#endif
