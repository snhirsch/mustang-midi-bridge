// -*-c++-*-

#ifndef _AMPCC_H
#define _AMPCC_H

#include <cstring>

class Mustang;

// F57 Deluxe
// F57 Champ
// F65 Deluxe
// F65 Princeton
// 60s Thrift
//
class AmpCC {

protected:
  Mustang * amp;
  unsigned char model[2];
  unsigned char slot;

  int continuous_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd );
  int discrete_control( int parm5, int parm6, int parm7, int value, unsigned char *cmd );

public:
  AmpCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : 
    amp(theAmp), 
    slot(theSlot) 
  {
    memcpy( this->model, model, 2 );
  }

  int dispatch( int cc, int value, unsigned char *cmd );
  const unsigned char *getModel( void ) { return model;}
  const unsigned char getSlot( void ) { return slot;}

private:
  // Gain
  virtual int cc69( int value, unsigned char *cmd ) { return continuous_control( 0x01, 0x01, 0x0c, value, cmd );}
  // Ch. Volume
  virtual int cc70( int value, unsigned char *cmd ) { return continuous_control( 0x00, 0x00, 0x0c, value, cmd );}
  // Treble
  virtual int cc71( int value, unsigned char *cmd ) { return continuous_control( 0x04, 0x04, 0x0c, value, cmd );}
  // Mid
  virtual int cc72( int value, unsigned char *cmd ) { return continuous_control( 0x05, 0x05, 0x0c, value, cmd );}
  // Bass
  virtual int cc73( int value, unsigned char *cmd ) { return continuous_control( 0x06, 0x06, 0x0c, value, cmd );}
  // Sag
  virtual int cc74( int value, unsigned char *cmd ) { 
    if ( value <= 2 ) return discrete_control( 0x13, 0x13, 0x8f, value, cmd );
    else              return -1;
  }
  // Bias
  virtual int cc75( int value, unsigned char *cmd ) { return continuous_control( 0x0a, 0x0a, 0x0d, value, cmd );}
  // Noise Gate
  virtual int cc76( int value, unsigned char *cmd ) { 
    if ( value <= 4 ) return discrete_control( 0x0f, 0x0f, 0x90, value, cmd );
    else              return -1;
  }
  // Cabinet
  virtual int cc77( int value, unsigned char *cmd ) {
    if ( value <= 12 ) return discrete_control( 0x11, 0x11, 0x8e, value, cmd );
    else               return -1;
  }

  // Dummy in base class
  virtual int cc78( int value, unsigned char *cmd ) { return -1;}
  virtual int cc79( int value, unsigned char *cmd ) { return -1;}

  // Noise Gate Custom Threshold
  virtual int cc90( int value, unsigned char *cmd ) { 
    if ( value <= 9 ) return discrete_control( 0x10, 0x10, 0x86, value, cmd );
    else              return -1;
  }
  // Noise Gate Custom Depth
  virtual int cc91( int value, unsigned char *cmd ) { return continuous_control( 0x09, 0x09, 0x0c, value, cmd );}
  // Dummy in base class
  virtual int cc92( int value, unsigned char *cmd ) { return -1;}
};


// F59 Bassman
// British 70s
//
class AmpCC1 : public AmpCC {
public:
  AmpCC1( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // Presence
  virtual int cc78( int value, unsigned char *cmd ) { return continuous_control( 0x07, 0x07, 0x0c, value, cmd );}
  // Blend
  virtual int cc79( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x0c, value, cmd );}
};
  

// Fender Supersonic
//
class AmpCC2 : public AmpCC {
public:
  AmpCC2( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // Gain2
  virtual int cc78( int value, unsigned char *cmd ) { return continuous_control( 0x02, 0x02, 0x0c, value, cmd );}
  // Master Volume
  virtual int cc79( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x0c, value, cmd );}
};
  

// British 60s
//
class AmpCC3 : public AmpCC {
public:
  AmpCC3( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // Cut
  virtual int cc78( int value, unsigned char *cmd ) { return continuous_control( 0x07, 0x07, 0x0c, value, cmd );}
  // Master Volume
  virtual int cc79( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x0c, value, cmd );}
  // Bright Switch
  virtual int cc92( int value, unsigned char *cmd ) { 
    // Inverted logic
    unsigned char flag;
    // 0 --> Bright On
    if ( value>63 && value <=127 ) flag = 0;
    else                           flag = 1;
    return discrete_control( 0x14, 0x14, 0x8d, value, cmd );
  }
};
  

// British 80s
// American 90s
// Metal 2000
// British Watt
//
class AmpCC4 : public AmpCC {
public:
  AmpCC4( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // Presence
  virtual int cc78( int value, unsigned char *cmd ) { return continuous_control( 0x07, 0x07, 0x0c, value, cmd );}
  // Master Volume
  virtual int cc79( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x0c, value, cmd );}
};
  

// Studio Preamp
//
class AmpCC5 : public AmpCC {
public:
  AmpCC5( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // No sag / bias
  virtual int cc74( int value, unsigned char *cmd ) { return -1;}
  virtual int cc75( int value, unsigned char *cmd ) { return -1;}
  // No pres / master
  virtual int cc78( int value, unsigned char *cmd ) { return -1;}
  virtual int cc79( int value, unsigned char *cmd ) { return -1;}
};
  

// British Color
//
class AmpCC6 : public AmpCC {
public:
  AmpCC6( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {;}
private:
  // Master Volume
  virtual int cc79( int value, unsigned char *cmd ) { return continuous_control( 0x03, 0x03, 0x0c, value, cmd );}
};
  

// F57 Twin
//
class AmpCC7 : public AmpCC {
public:
  AmpCC7( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // Presence
  virtual int cc78( int value, unsigned char *cmd ) { return continuous_control( 0x07, 0x07, 0x0c, value, cmd );}
};
  

// F65 Twin
//
class AmpCC8 : public AmpCC {
public:
  AmpCC8( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  // Bright Switch
  virtual int cc92( int value, unsigned char *cmd ) { 
    // Inverted logic
    unsigned char flag;
    // 0 --> Bright On
    if ( value>63 && value <=127 ) flag = 0;
    else                           flag = 1;
    return discrete_control( 0x14, 0x14, 0x8d, value, cmd );
  }
};
  

// Null Amp
//
class NullAmpCC : public AmpCC {
public:
  NullAmpCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : AmpCC(theAmp,model,theSlot) {}
private:
  virtual int cc69( int value, unsigned char *cmd ) { return -1;}
  virtual int cc70( int value, unsigned char *cmd ) { return -1;}
  virtual int cc71( int value, unsigned char *cmd ) { return -1;}
  virtual int cc72( int value, unsigned char *cmd ) { return -1;}
  virtual int cc73( int value, unsigned char *cmd ) { return -1;}
  virtual int cc74( int value, unsigned char *cmd ) { return -1;}
  virtual int cc75( int value, unsigned char *cmd ) { return -1;}
  virtual int cc76( int value, unsigned char *cmd ) { return -1;}
  virtual int cc77( int value, unsigned char *cmd ) { return -1;}
  virtual int cc78( int value, unsigned char *cmd ) { return -1;}
  virtual int cc79( int value, unsigned char *cmd ) { return -1;}
};
  
#endif
