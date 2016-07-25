// -*-c++-*-

#ifndef _DELAY_H
#define _DELAY_H

#include <cstring>

class Mustang;

class DelayCC {

protected:
  Mustang * amp;
  unsigned char model[2];
  unsigned char slot;

  int continuous_control( int parm5, int parm6, int parm7, int value );
  int discrete_control( int parm5, int parm6, int parm7, int value );

public:
  DelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : 
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
  virtual int cc49( int value ) { return continuous_control( 0x00, 0x00, 0x01, value );}
  // Delay Time
  virtual int cc50( int value ) { return continuous_control( 0x01, 0x01, 0x06, value );}
  
  virtual int cc51( int value ) = 0;
  virtual int cc52( int value ) = 0;
  virtual int cc53( int value ) = 0;
  virtual int cc54( int value ) = 0;
};


class MonoDelayCC : public DelayCC {
public:
  MonoDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Brightness
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Attenuation
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
  // no-op
  virtual int cc54( int value ) { return 0;}
};


class EchoFilterCC : public DelayCC {
public:
  EchoFilterCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Frequency
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Resonance
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
  // Input Level
  virtual int cc54( int value ) { return continuous_control( 0x05, 0x05, 0x01, value );}
};


class MultitapDelayCC : public DelayCC {
public:
  MultitapDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Delay Time
  virtual int cc50( int value ) { return continuous_control( 0x01, 0x01, 0x08, value );}
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Brightness
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Mode
  virtual int cc53( int value ) { 
    if ( value > 3 ) return 0;
    else             return discrete_control( 0x04, 0x04, 0x8b, value );
  }
  // no-op
  virtual int cc54( int value ) { return 0;}
};


class PingPongDelayCC : public DelayCC {
public:
  PingPongDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Brightness
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Stereo
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
  // no-op
  virtual int cc54( int value ) { return 0;}
};


class DuckingDelayCC : public DelayCC {
public:
  DuckingDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Release
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Threshold
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
  // no-op
  virtual int cc54( int value ) { return 0;}
};


class ReverseDelayCC : public DelayCC {
public:
  ReverseDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // FFdbk
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // RFdbk
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Tone
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
  // no-op
  virtual int cc54( int value ) { return 0;}
};


class TapeDelayCC : public DelayCC {
public:
  TapeDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Flutter
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Brightness
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x04, 0x01, value );}
  // Stereo
  virtual int cc54( int value ) { return continuous_control( 0x05, 0x05, 0x01, value );}
};


class StereoTapeDelayCC : public DelayCC {
public:
  StereoTapeDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  // Feedback
  virtual int cc51( int value ) { return continuous_control( 0x02, 0x02, 0x01, value );}
  // Flutter
  virtual int cc52( int value ) { return continuous_control( 0x03, 0x03, 0x01, value );}
  // Separation
  virtual int cc53( int value ) { return continuous_control( 0x04, 0x05, 0x01, value );}
  // Brightness
  virtual int cc54( int value ) { return continuous_control( 0x05, 0x04, 0x01, value );}
};


class NullDelayCC : public DelayCC {
public:
  NullDelayCC( Mustang * theAmp, const unsigned char *model, const unsigned char theSlot ) : DelayCC(theAmp,model,theSlot) {}
private:
  virtual int cc49( int value ) { return 0;}
  virtual int cc50( int value ) { return 0;}
  virtual int cc51( int value ) { return 0;}
  virtual int cc52( int value ) { return 0;}
  virtual int cc53( int value ) { return 0;}
  virtual int cc54( int value ) { return 0;}
};


#endif
